#include <utils.h>

void read_file(std::string file_name)
{
	std::ifstream fin (file_name);
	if (!fin)
	{
        std::cout << "read file error!" << std::endl;
        exit(1);
	}
	char buf[128];
	fin.getline (buf, sizeof(buf));  /* neglect table header */
	int i, j, mem;
	std::string header, num, reg, val;
	/* store constants into CONS_MAP */
	for (i = 0; i < 3; ++i)
	{
        fin >> header >> header;
    for (j = 0; j < 4; ++j)
        {
            if (j == 2)
            {
                CONS_MAP[i][2] = 0;
                continue;
            }
            fin >> num;
            CONS_MAP[i][j] = atoi(num.c_str());
        }
	}
	fin >> header >> header;   /* Load/store unit */
	for (j = 0; j < 4; ++j)
	{
        fin >> num;
        CONS_MAP[3][j] = atoi(num.c_str());
	}

	/* store ROB size into ROB_SIZE */
	fin >> header >> header >> header; /* ROB entries = 128 */
	fin >> num;
	ROB_SIZE = atoi(num.c_str());

	/* store all register into REG */
	fin.ignore(std::numeric_limits<std::streamsize>::max(),'\n'); /* move pointer to next line */
	fin.getline(buf, sizeof(buf));

	std::vector<std::string> vec = split(buf, ',');
	for (i = 0; i < vec.size(); ++i)
	{
        int eqloc = vec[i].find("=");  /* location of euqal sign */
        val = vec[i].substr(eqloc + 1);
        reg = vec[i].substr(0, vec[i].length() - 1 - val.length());
        // std::cout << val << "  " << reg << std::endl;
        std::pair<std::string, float> the_pair (reg, atof(val.c_str()));
        REG.insert(the_pair);
	}

	/* store all memory address and value into MEM */
	fin.getline(buf, sizeof(buf));
	vec = split(buf, ',');
	for (i = 0; i < vec.size(); ++i)
	{
        int eqloc = vec[i].find("=");  /* location of euqal sign */
        int lblloc = vec[i].find("]");
        int rblloc = vec[i].find("[");
        val = vec[i].substr(eqloc + 1);
        reg = vec[i].substr(4, rblloc - lblloc);
        std::pair<int, float> the_pair (atoi(reg.c_str()), atof(val.c_str()));
        MEM.insert(the_pair);
	}

	/* store all instructions into ALL_INS */
	do {
        fin.getline(buf, sizeof(buf));
	} while (strcmp(buf, "") == 0);
	int ins_order = 1;
	++INS_NUM;
	std::pair<int, std::string> the_ins (ins_order++, buf);
	ALL_INS.insert(the_ins);
	while (fin.getline(buf, sizeof(buf)))
	{
        ++INS_NUM;
        std::pair<int, std::string> the_ins (ins_order++, buf);
        ALL_INS.insert(the_ins);
	}
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        elems.push_back(trim(item));
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

static inline std::string &ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}


static inline std::string &rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}


static inline std::string &trim(std::string &s)
{
    return ltrim(rtrim(s));
}

void pro_instr(const std::string &s, std::vector<std::string> &elems)
{
	std::stringstream ss(s);
    std::string item;
    int i = 0;
    while (std::getline(ss, item, ','))
    {
    	if (i == 0)
    	{
    		std::string op, reg1;
    		item = trim(item);
    		int blkloc = item.find(" ");
    		reg1 = item.substr(blkloc + 1);
    		op = item.substr(0, blkloc);
    		elems.push_back(op);
    		elems.push_back(reg1);
    	}
    	else
    	{
    		elems.push_back(trim(item));
    	}
    	++i;
    }
}

void init_all()
{
	int i; /* for loop counting */
	int j; /* for creating rs */
	/* result table */
	/* Init ROB */
	j = ROB_SIZE;
	ROB = new struct rob_entry *[j];
	for (i = 0; i < j; ++i)
	{
        ROB[i] = new struct rob_entry;
        ROB[i]->rob_name = "ROB" + std::to_string(i + 1);
        ROB[i]->finish = FALSE;
	}



	/* Integer adder RS */
	j = CONS_MAP[INTEGER_ADDER][NUM_RS];
	INTEGER_ADDER_RS = new struct RS *[j];
	for (i = 0; i < j; ++i)
	{
		INTEGER_ADDER_RS[i] = new struct RS;
		INTEGER_ADDER_RS[i]->BUSY = FALSE;
		INTEGER_ADDER_RS[i]->VJ = EMPTY;
		INTEGER_ADDER_RS[i]->VK = EMPTY;
		INTEGER_ADDER_RS[i]->A = EMPTY;
	}

	/* FP adder RS */
	j = CONS_MAP[FP_ADDER][NUM_RS];
	FP_ADDER_RS = new struct RS *[j];

	for (i = 0; i < j; ++i)
	{
		FP_ADDER_RS[i] = new struct RS;
		FP_ADDER_RS[i]->BUSY = FALSE;
		FP_ADDER_RS[i]->VJ = EMPTY;
		FP_ADDER_RS[i]->VK = EMPTY;
		FP_ADDER_RS[i]->A = EMPTY;
	}

	/* FP multiplier RS */
	j = CONS_MAP[FP_MULTIPLIER][NUM_RS];
	FP_MULT_RS = new struct RS *[j];
	for (i = 0; i < j; ++i)
	{
		FP_MULT_RS[i] = new struct RS;
		FP_MULT_RS[i]->BUSY = FALSE;
		FP_MULT_RS[i]->VJ = EMPTY;
		FP_MULT_RS[i]->VK = EMPTY;
		FP_MULT_RS[i]->A = EMPTY;
	}

	/* Load/store unit RS */
	j = CONS_MAP[LOAD_STORE_UNIT][NUM_RS];
	LS_RS = new struct RS *[j];
	for (i = 0; i < j; ++i)
	{
		LS_RS[i] = new struct RS;
		LS_RS[i]->BUSY = FALSE;
		LS_RS[i]->VJ = EMPTY;
		LS_RS[i]->VK = EMPTY;
		LS_RS[i]->A = EMPTY;
	}

	/* init ARF */
	for (std::unordered_map<std::string, float>::iterator it = REG.begin();
			it != REG.end(); ++it)
	{
		// if ((it->first).substr(0, 1).compare("R") != 0)
		// {
		std::pair<std::string, float> the_F (it->first, it->second);
		ARF.insert(the_F);
		// }
	}
}

int fetch_ins(struct instr& INS)
{
	std::string ins = INS._ins;
	int order = INS.num;
	int has_fetch = FALSE;
	int i; /* for loop counting */
	int j; /* for RS counting */
	std::vector<std::string> elems;
	pro_instr(ins, elems);
	/**
	 *  busy, op, vj, vk, qj, qk, a, rat, arf
	 */
	std::string op = elems[0];
	std::string operant1 = elems[1];
	std::string operant2 = elems[2];
	/* case Ld or Sd */
	int loc1 = operant2.find("(");
	int loc2 = operant2.find(")");
	int num = atoi(operant2.substr(0, loc1).c_str());
	std::string reg = operant2.substr(loc1 + 1, loc2 - loc1 - 1);

	if (op == "Ld")
	{

		j = CONS_MAP[LOAD_STORE_UNIT][NUM_RS];
		for (i = 0; i < j; ++i)
		{
			if (LS_RS[i]->BUSY == FALSE)
			{
				LS_RS[i]->BUSY = TRUE;
				LS_RS[i]->OP = op;
				LS_RS[i]->ROB_ENTRY = "ROB" + std::to_string(ROB_NOW_NUM);
				INS.in_rs = i;
				update_rat_rob(operant1);
				if (RAT.find(reg) == RAT.end())  /* reg not rename in RAT */
				{
					std::cout << "not renamed in RAT" << reg << std::endl;
					LS_RS[i]->A = (num + ARF[reg]);
				}
				else if (ROB[RAT[reg] - 1]->finish == TRUE)
				{
					LS_RS[i]->A = (num + ROB[RAT[reg] - 1]->value);
				}
				else
				{
					LS_RS[i]->QK = ROB[RAT[reg] - 1]->rob_name;
					LS_RS[i]->VK = num;
				}
				has_fetch = TRUE;
				++LS_RS_USED;
				break;
			}
		}
	}

	else if (op == "Sd")
	{
		j = CONS_MAP[LOAD_STORE_UNIT][NUM_RS];
		for (i = 0; i < j; ++i)
		{
			if (LS_RS[i]->BUSY == FALSE)
			{
				LS_RS[i]->BUSY = TRUE;
				LS_RS[i]->OP = op;
				LS_RS[i]->ROB_ENTRY = "ROB" + std::to_string(ROB_NOW_NUM);
				++ROB_NOW_NUM;
				INS.in_rs = i;
				// update_rat_rob(operant1);
				if (RAT.find(reg) == RAT.end())  /* reg not rename in RAT */
				{
					// std::cout << "&&&*********************** here" << std::endl;
					LS_RS[i]->A = (num + ARF[reg]);
				}
				else if (ROB[RAT[reg] - 1]->finish == TRUE)
				{
					LS_RS[i]->A = (num + ROB[RAT[reg] - 1]->value);
				}
				else
				{
					LS_RS[i]->QK = ROB[RAT[reg] - 1]->rob_name;
					LS_RS[i]->VK = num;
				}
				std::pair<std::string, int> the_pair (operant2, TRUE);
				MEMORY_LOCK.insert(the_pair);
				std::cout << "debugging here ************** " << operant2 << " locked" << std::endl;

				if (RAT.find(operant1) == RAT.end())
				{
					LS_RS[i]->VJ = ARF[operant1];
				}
				else if (ROB[RAT[operant1] - 1]->finish == TRUE)
				{
					LS_RS[i]->VJ = ROB[RAT[operant1] - 1]->value;
				}
				else
				{
					LS_RS[i]->QJ = ROB[RAT[operant1] - 1]->rob_name;
				}
				has_fetch = TRUE;
				++LS_RS_USED;
				break;
			}
		}
	}
	else if (op == "Beq" || op == "Bne")
	{
		std::string operant3 = elems[3];
		j = CONS_MAP[INTEGER_ADDER][NUM_RS];
		for (i = 0; i < j; ++i)
		{
			if (INTEGER_ADDER_RS[i]->BUSY == FALSE)
			{
				INTEGER_ADDER_RS[i]->BUSY = TRUE;
				INTEGER_ADDER_RS[i]->OP = op;
				INTEGER_ADDER_RS[i]->ROB_ENTRY = "ROB" + std::to_string(ROB_NOW_NUM);
				// std::cout << "fetch lalallal " << INTEGER_ADDER_RS[i]->ROB_ENTRY << std::endl;
				set_operant(operant1, 0, i, 1);
				set_operant(operant2, 0, i, 2);
				INTEGER_ADDER_RS[i]->A = atoi(operant3.c_str());
				update_rat_rob(op);
				INS.in_rs = i;
				has_fetch = TRUE;
				++INTEGER_ADDER_RS_USED;
				branch_stall = TRUE;
				break;
			}
		}
	}
	else
	{
		std::string operant3 = elems[3];
		/* three situation.  (Add, Addi, Sub), (Add.d, Sub.d), (Mul.d) */
		if (op == "Addi")
		{
			j = CONS_MAP[INTEGER_ADDER][NUM_RS];
			for (i = 0; i < j; ++i)
			{
				if (INTEGER_ADDER_RS[i]->BUSY == FALSE)
				{
					INTEGER_ADDER_RS[i]->BUSY = TRUE;
					INTEGER_ADDER_RS[i]->OP = op;
					INTEGER_ADDER_RS[i]->ROB_ENTRY = "ROB" + std::to_string(ROB_NOW_NUM);
					set_operant(operant2, 0, i, 1);
					INTEGER_ADDER_RS[i]->VK = atoi(operant3.c_str());
					update_rat_rob(operant1);
					INS.in_rs = i;
					has_fetch = TRUE;
					++INTEGER_ADDER_RS_USED;
					break;
				}
			}
		}
		else if (op == "Add.d" || op == "Sub.d")
		{
			j = CONS_MAP[FP_ADDER][NUM_RS];
			for (i = 0; i < j; ++i)
			{
				if (FP_ADDER_RS[i]->BUSY == FALSE)
				{
					FP_ADDER_RS[i]->BUSY = TRUE;
					FP_ADDER_RS[i]->OP = op;
					FP_ADDER_RS[i]->ROB_ENTRY = "ROB" + std::to_string(ROB_NOW_NUM);
					set_operant(operant2, 1, i, 1);
					set_operant(operant3, 1, i, 2);
					update_rat_rob(operant1);
					INS.in_rs = i;
					has_fetch = TRUE;
					++FP_ADDER_RS_USED;
					break;
				}
			}
		}
		else if (op == "Mult.d")
		{
			j = CONS_MAP[FP_MULTIPLIER][NUM_RS];
			for (i = 0; i < j; ++i)
			{
				if (FP_MULT_RS[i]->BUSY == FALSE)
				{
					FP_MULT_RS[i]->BUSY = TRUE;
					FP_MULT_RS[i]->OP = op;
					FP_MULT_RS[i]->ROB_ENTRY = "ROB" + std::to_string(ROB_NOW_NUM);
					set_operant(operant2, 2, i, 1);
					// std::cout << "operant3 is: " << operant3 << std::endl;

					set_operant(operant3, 2, i, 2);
					update_rat_rob(operant1);
					INS.in_rs = i;
					has_fetch = TRUE;
					++FP_MULT_RS_USED;
					break;
				}
			}
		}
		else
		{
			j = CONS_MAP[INTEGER_ADDER][NUM_RS];
			for (i = 0; i < j; ++i)
			{
				if (INTEGER_ADDER_RS[i]->BUSY == FALSE)
				{
					INTEGER_ADDER_RS[i]->BUSY = TRUE;
					INTEGER_ADDER_RS[i]->OP = op;
					INTEGER_ADDER_RS[i]->ROB_ENTRY = "ROB" + std::to_string(ROB_NOW_NUM);
					set_operant(operant2, 0, i, 1);
					set_operant(operant3, 0, i, 2);
					update_rat_rob(operant1);
					INS.in_rs = i;
					has_fetch = TRUE;
					++INTEGER_ADDER_RS_USED;
					break;
				}
			}
		}
	}
	return has_fetch;
}

int cal_addr(struct instr& INS)
{
	int res;
	if (LS_RS[INS.in_rs]->A != EMPTY)
	{
		res = LS_RS[INS.in_rs]->A;
		return res;
	}
	else
	{
		res = int(ROB[atoi(LS_RS[INS.in_rs]->QK.substr(3, 1).c_str()) - 1]->value + LS_RS[INS.in_rs]->VK);
		return res;
	}
}

int store_memory_just_commit(int address)
{
	std::vector<int>::iterator it;
	it = find(JUST_COMMIT_ADDR.begin(), JUST_COMMIT_ADDR.end(), address);
	if (it != JUST_COMMIT_ADDR.end())
		return TRUE;
	else
		return FALSE;
}


int addr_ready(struct instr& INS)
{
	int loc1 = INS._ins.find("(");
	int loc2 = INS._ins.find(")");
	std::string reg = INS._ins.substr(loc1 + 1, loc2 - loc1 - 1);
	if (LS_RS[INS.in_rs]->A != EMPTY)
	{
		if (store_memory_just_commit(LS_RS[INS.in_rs]->A) == FALSE)
			return TRUE;
		else
			return FALSE;
	}
	else if (ROB[atoi(LS_RS[INS.in_rs]->QK.substr(3, 1).c_str()) - 1]->finish == TRUE && TEM_REG_LOCKER.find(reg) == TEM_REG_LOCKER.end())
	{
		int address = int(ROB[atoi(LS_RS[INS.in_rs]->QK.substr(3, 1).c_str()) - 1]->value + LS_RS[INS.in_rs]->VK);
		if (store_memory_just_commit(address) == FALSE)
			return TRUE;
		else return FALSE;
	}
	else
	{
		return FALSE;
	}
}

void set_operant(std::string operant, int type, int num, int operant_num)
{
	// std::cout << "in ARF: " << ARF[operant] << std::endl;
	// std::cout << "now the operant is: " << operant << std::endl;
	// std::cout << operant << " in RAT is: " << RAT[operant] << std::endl;
	// std::cout << operant << " in ARF is: " << ARF[operant] << std::endl;
	if (RAT.find(operant) == RAT.end())  /* not rename in RAT, then use value in ARF */
	{
		// std::cout << operant << " not found in rat" << std::endl;
		if (type == 0)
		{
			if (operant_num == 1)
				INTEGER_ADDER_RS[num]->VJ = ARF[operant];
			else
				INTEGER_ADDER_RS[num]->VK = ARF[operant];
		}
		else if (type == 1)
		{
			if (operant_num == 1)
				FP_ADDER_RS[num]->VJ = ARF[operant];
			else
				FP_ADDER_RS[num]->VK = ARF[operant];
		}
		else
		{
			if (operant_num == 1)
			{
				// std::cout << "in if" << std::endl;
				FP_MULT_RS[num]->VJ = ARF[operant];
				// std::cout << "VJC: " << FP_MULT_RS[0]->VJ << std::endl;
			}
			else
			{
				// std::cout << "in else" << std::endl;
				FP_MULT_RS[num]->VK = ARF[operant];
				// std::cout << "VJD: " << FP_MULT_RS[0]->VK << std::endl;
			}
		}
	}
	else /* remember to minus 1 */
	{
		// std::cout << operant << " found in rat" << std::endl;
		if (ROB[RAT[operant] - 1]->finish == TRUE && TEM_REG_LOCKER.find(operant) == TEM_REG_LOCKER.end())
		{
			if (type == 0)
			{
				if (operant_num == 1)
					INTEGER_ADDER_RS[num]->VJ = ROB[RAT[operant] - 1]->value;
				else
					INTEGER_ADDER_RS[num]->VK = ROB[RAT[operant] - 1]->value;
			}
			else if (type == 1)
			{
				if (operant_num == 1)
					FP_ADDER_RS[num]->VJ = ROB[RAT[operant] - 1]->value;
				else
					FP_ADDER_RS[num]->VK = ROB[RAT[operant] - 1]->value;
			}
			else
			{
				if (operant_num == 1)
					FP_MULT_RS[num]->VJ = ROB[RAT[operant] - 1]->value;
				else
					FP_MULT_RS[num]->VK = ROB[RAT[operant] - 1]->value;
			}
		}
		else
		{
			if (type == 0)
			{
				if (operant_num == 1)
					INTEGER_ADDER_RS[num]->QJ = "ROB" + std::to_string(RAT[operant]);
				else
					INTEGER_ADDER_RS[num]->QK = "ROB" + std::to_string(RAT[operant]);
			}
			else if (type == 1)
			{
				if (operant_num == 1)
					FP_ADDER_RS[num]->QJ = "ROB" + std::to_string(RAT[operant]);
				else
					FP_ADDER_RS[num]->QK = "ROB" + std::to_string(RAT[operant]);
			}
			else
			{
				if (operant_num == 1)
					FP_MULT_RS[num]->QJ = "ROB" + std::to_string(RAT[operant]);
				else
					FP_MULT_RS[num]->QK = "ROB" + std::to_string(RAT[operant]);
			}
		}

	}
}

void print_rs()
{
	std::cout << "Busy" << "		" << "OP" << "		" << "ROB" << "		" << "VJ" << "		" << "VK" <<
	"		" << "QJ" << "		" << "QK" << "		" << "A" << std::endl;
	int i, j;
	j = CONS_MAP[INTEGER_ADDER][NUM_RS];
	for (i = 0; i < j; ++i)
	{
		std::cout << INTEGER_ADDER_RS[i]->BUSY << "		" <<
					 INTEGER_ADDER_RS[i]->OP << "		" <<
					 INTEGER_ADDER_RS[i]->ROB_ENTRY << "		" <<
					 INTEGER_ADDER_RS[i]->VJ << "		" <<
					 INTEGER_ADDER_RS[i]->VK << "		" <<
					 INTEGER_ADDER_RS[i]->QJ << "		" <<
					 INTEGER_ADDER_RS[i]->QK << "		" <<
					 INTEGER_ADDER_RS[i]->A << "		" << std::endl;
  	}
  	std::cout << std::endl;

  	j = CONS_MAP[FP_ADDER][NUM_RS];
	for (i = 0; i < j; ++i)
	{
		std::cout << FP_ADDER_RS[i]->BUSY << "		" <<
					 FP_ADDER_RS[i]->OP << "		" <<
					 FP_ADDER_RS[i]->ROB_ENTRY << "		" <<
					 FP_ADDER_RS[i]->VJ << "		" <<
					 FP_ADDER_RS[i]->VK << "		" <<
					 FP_ADDER_RS[i]->QJ << "		" <<
					 FP_ADDER_RS[i]->QK << "		" <<
					 FP_ADDER_RS[i]->A << "		" << std::endl;
  	}
  	std::cout << std::endl;

  	j = CONS_MAP[FP_MULTIPLIER][NUM_RS];
	for (i = 0; i < j; ++i)
	{
		std::cout << FP_MULT_RS[i]->BUSY << "		" <<
					 FP_MULT_RS[i]->OP << "		" <<
					 FP_MULT_RS[i]->ROB_ENTRY << "		" <<
					 FP_MULT_RS[i]->VJ << "		" <<
					 FP_MULT_RS[i]->VK << "		" <<
					 FP_MULT_RS[i]->QJ << "		" <<
					 FP_MULT_RS[i]->QK << "		" <<
					 FP_MULT_RS[i]->A << "		" << std::endl;
  	}
  	std::cout << std::endl;

  	j = CONS_MAP[LOAD_STORE_UNIT][NUM_RS];
	for (i = 0; i < j; ++i)
	{
		std::cout << LS_RS[i]->BUSY << "		" <<
					 LS_RS[i]->OP << "		" <<
					 LS_RS[i]->ROB_ENTRY << "		" <<
					 LS_RS[i]->VJ << "		" <<
					 LS_RS[i]->VK << "		" <<
					 LS_RS[i]->QJ << "		" <<
					 LS_RS[i]->QK << "		" <<
					 LS_RS[i]->A << "		" << std::endl;
  	}
  	std::cout << std::endl;
}

void print_rat()
{
	std::cout << "********** RAT **********" << std::endl;
	for (std::unordered_map<std::string, int>::iterator it = RAT.begin();
			it != RAT.end(); ++it)
	{
		std::cout << it->first << "		" << it->second << std::endl;
	}
	std::cout << "*************************" << std::endl;
	std::cout << std::endl;
}

void print_memory()
{
	std::cout << "********** MEM **********" << std::endl;
	for (std::unordered_map<int, float>::iterator it = MEM.begin();
			it != MEM.end(); ++it)
	{
		std::cout << it->first << "		" << it->second << std::endl;
	}
	std::cout << "*************************" << std::endl;
	std::cout << std::endl;
}

void print_arf()
{
	std::cout << "********** ARF **********" << std::endl;
	for (std::unordered_map<std::string, float>::iterator it = ARF.begin();
			it != ARF.end(); ++it)
	{
		std::cout << it->first << "		" << it->second << std::endl;
	}
	std::cout << "*************************" << std::endl;
	std::cout << std::endl;
}

void init_instructions()
{
	int i;
	// std::cout << "instruction number is: " << INS_NUM << std::endl;
	// INSTRS = new struct instr *[INS_NUM];
	for (i = 0; i < INS_NUM; ++i)
	{
		struct instr tmp_create;
		std::string the_ins = ALL_INS[i + 1];
		tmp_create._ins = the_ins;
		tmp_create.num = i + 1;
		tmp_create.in_rs = EMPTY;
		tmp_create.state = -1;
		tmp_create.ex_begin = 0;
		tmp_create.mem_begin = 0;
		tmp_create.has_committed = FALSE;
		tmp_create.inQ = 0;
		if (the_ins.find("Ld") == 0 || the_ins.find("Sd") == 0)
		{
			// std::cout << "in Ld, Sd" << std::endl;
			tmp_create.ins_type = LOAD_STORE_UNIT;
			tmp_create.cycle_need = CONS_MAP[LOAD_STORE_UNIT][CYCLE_MEM];
			tmp_create.has_committed = FALSE;
		}
		else if (the_ins.find("Add.d") == 0 || the_ins.find("Sub.d") == 0)
		{
			// std::cout << "in Add.d, Sub.d" << std::endl;
			tmp_create.ins_type = FP_ADDER;
			tmp_create.cycle_need = CONS_MAP[FP_ADDER][CYCLE_EX];
			tmp_create.has_committed = FALSE;
		}
		else if (the_ins.find("Mult.d") == 0)
		{
			// std::cout << "in mult.d" << std::endl;
			tmp_create.ins_type = FP_MULTIPLIER;
			tmp_create.cycle_need = CONS_MAP[FP_MULTIPLIER][CYCLE_EX];
			tmp_create.has_committed = FALSE;
		}
		else
		{
			// std::cout << "in other" << std::endl;
			tmp_create.ins_type = INTEGER_ADDER;
			// std::cout << "type is: " << INTEGER_ADDER << std::endl;
			tmp_create.cycle_need = CONS_MAP[INTEGER_ADDER][CYCLE_EX];
			tmp_create.has_committed = FALSE;
			// std::cout << "cycle needed is: " << CONS_MAP[INTEGER_ADDER][CYCLE_EX] << std::endl;
		}
		INSTRS.push_back(tmp_create);
	}
}

int check_rs_available(struct instr& INS)
{
	// std::cout << INSTRS[num]->ins_type << std::endl;
	// std::cout << INSTRS[num]->_ins << std::endl;
	if (INS.ins_type == INTEGER_ADDER)
	{
		if (INTEGER_ADDER_RS_USED == CONS_MAP[INTEGER_ADDER][NUM_RS])
			return FALSE;
		else
			return TRUE;
	}
	else if (INS.ins_type == FP_ADDER)
	{
		if (FP_ADDER_RS_USED == CONS_MAP[FP_ADDER][NUM_RS])
			return FALSE;
		else
			return TRUE;
	}
	else if (INS.ins_type == FP_MULTIPLIER)
	{
		if (FP_MULT_RS_USED == CONS_MAP[FP_MULTIPLIER][NUM_RS])
			return FALSE;
		else
			return TRUE;
	}
	else
	{
		if (LS_RS_USED == CONS_MAP[LOAD_STORE_UNIT][NUM_RS])
			return FALSE;
		else
			return TRUE;
	}
}

int issue(struct instr& INS)
{
	int num = INS.num;
	std::cout << "Instruction " << num << " state is: " << INS.state << std::endl;
	if (INS.state == -1)
	{
		
		int tmp = check_rs_available(INS);
		if (tmp == TRUE) /* entry available for this instruction */
		{
			/* remember to minus one */
			// RESULT[SHOULD_FETCH - 1][ISSUE] = CYCLE;
			// std::cout << RESULT[SHOULD_FETCH - 1][ISSUE] << std::endl;
			// std::cout << INSTRS[num - 1]->_ins << std::endl;
			// std::cout << renamed_in_rat(first_operant(INSTRS[num - 1]->_ins)) << std::endl;
				// std::cout << "************** message for debugging ************** "<< RAT[first_operant(INSTRS[num - 1]->_ins)] << std::endl;
				// std::cout << "here" << std::endl;
			int has_fetch = fetch_ins(INS);
			// std::cout << "here: " << has_fetch << std::endl;
			std::pair<int, int> the_pair (num, has_fetch);
			HAS_FETCH.insert(the_pair);
			// ++SHOULD_FETCH;
			std::cout << "Instruction " << num << " issue" << std::endl;
			// std::cout << "******** &&&&&&&& ^^^^^^^^ " << INS_QUEUE[INS_QUEUE.size() - 1].state <<std::endl;
			INS.state = ISSUE;
			// std::cout << "debugging " << INS.state << "	" << SHOULD_FETCH << std::endl;
			return TRUE;
		}
		std::cout << num << " RS is full." << std::endl;
		return FALSE;
	}
	return TRUE;
}

int value_available_in_vk_or_qk(struct instr& INS)
{
	std::string ins = INS._ins;
	int _in_rs = INS.in_rs;
	if (ins.find("Mult.d") == 0)
	{
		if (FP_MULT_RS[_in_rs]->VK != EMPTY)
			return TRUE;
		else
		{
			int _in_rob = atoi(FP_MULT_RS[_in_rs]->QK.substr(3, 1).c_str());
			if (ROB[_in_rob - 1]->finish == TRUE)
				return TRUE;
			else
				return FALSE;
		}
	}
	else if (ins.find("Add.d") == 0 || ins.find("Sub.d") == 0)
	{
		if (FP_ADDER_RS[_in_rs]->VK != EMPTY)
			return TRUE;
		else
		{
			int _in_rob = atoi(FP_ADDER_RS[_in_rs]->QK.substr(3, 1).c_str());
			if (ROB[_in_rob - 1]->finish == TRUE)
				return TRUE;
			else
				return FALSE;
		}
	}
	else
	{
		if (INTEGER_ADDER_RS[_in_rs]->VK != EMPTY)
			return TRUE;
		else
		{
			int _in_rob = atoi(INTEGER_ADDER_RS[_in_rs]->QK.substr(3, 1).c_str());
			if (ROB[_in_rob - 1]->finish == TRUE)
				return TRUE;
			else
				return FALSE;
		}
	}
}

int value_available_in_vj_or_qj(struct instr& INS)
{
	std::string ins = INS._ins;
	int _in_rs = INS.in_rs;
	if (ins.find("Mult.d") == 0)
	{
		if (FP_MULT_RS[_in_rs]->VJ != EMPTY)
			return TRUE;
		else
		{
			int _in_rob = atoi(FP_MULT_RS[_in_rs]->QJ.substr(3, 1).c_str());
			if (ROB[_in_rob - 1]->finish == TRUE)
				return TRUE;
			else
				return FALSE;
		}
	}
	else if (ins.find("Add.d") == 0 || ins.find("Sub.d") == 0)
	{
		if (FP_ADDER_RS[_in_rs]->VJ != EMPTY)
			return TRUE;
		else
		{
			int _in_rob = atoi(FP_ADDER_RS[_in_rs]->QJ.substr(3, 1).c_str());
			if (ROB[_in_rob - 1]->finish == TRUE)
				return TRUE;
			else
				return FALSE;
		}
	}
	else
	{
		if (INTEGER_ADDER_RS[_in_rs]->VJ != EMPTY)
			return TRUE;
		else
		{
			int _in_rob = atoi(INTEGER_ADDER_RS[_in_rs]->QJ.substr(3, 1).c_str());
			if (ROB[_in_rob - 1]->finish == TRUE)
				return TRUE;
			else
				return FALSE;
		}
	}
}

std::string first_operant(std::string ins)
{
	int loc1 = ins.find(" ");
	int loc2 = ins.find(",");
	std::string operant = ins.substr(loc1 + 1, loc2 - loc1 - 1);
	return operant;
}

int execute(struct instr& INS)
{

	if (RESULT[INS.inQ - 1][ISSUE] == 0)
	{
		std::cout << "Instruction " << INS.num << " did not issue" << std::endl;
		return FALSE;
	}
	else
	{
		std::vector<std::string> elems;
		pro_instr(INS._ins, elems);
		std::string op = elems[0];
		std::string operant1 = elems[1];
		std::string operant2 = elems[2];
		std::cout << "debugging here ********0 " << op << "	" << operant1 << "	"  << operant2 << std::endl;
		if (op.find("Ld") == 0)
		{
			if (LS_FU_USED < CONS_MAP[LOAD_STORE_UNIT][NUM_FUS])
			{
				std::cout << "debugging here ********1 " << op << "	" << operant1 << "	"  << operant2 << std::endl;
				std::cout << op << "	" << operant1 << "	" << operant2 << std::endl;
				int _in_rs = INS.in_rs;
				// print_memory_lock();
				if (MEMORY_LOCK.find(operant2) == MEMORY_LOCK.end())
				{
					if (addr_ready(INS) == TRUE)
					{
						std::cout << "Instruction " << INS.num << " begin execute" << std::endl;
						++LS_FU_USED;
						RESULT[INS.inQ - 1][EXE] = CYCLE;
						INS.state = EXE;
						return TRUE;
					}
					else
					{
						std::cout << "Instruction " << INS.num << " Address not ready" << std::endl;
						return FALSE;
					}
				}
				else
				{
					std::cout << "Address " << operant2 << "has been locked" << std::endl;
				}
			}
			std::cout << "there is no rs load_store_fu for it" << std::endl;
			return FALSE;
		}
		else if (op.find("Sd") == 0)
		{
			if (LS_FU_USED < CONS_MAP[LOAD_STORE_UNIT][NUM_FUS])
			{
				std::cout << "debugging here ********2 " << op << "	" << operant1 << "	"  << operant2 << std::endl;
				int _in_rs = INS.in_rs;
				std::cout << _in_rs << " in_rs" << std::endl;
				// std::cout << op << "	" << operant1 << "	" << operant2 << std::endl;
				// std::cout << "*****************^^^^^^^^********************" << std::endl;
				int loc1 = operant2.find("(");
				int loc2 = operant2.find(")");
				std::string reg = operant2.substr(loc1 + 1, loc2 - loc1 - 1);
				if (addr_ready(INS) == TRUE && (LS_RS[INS.in_rs]->VJ != EMPTY ||
					ROB[atoi(LS_RS[INS.in_rs]->QJ.substr(3, 1).c_str()) - 1]->finish == TRUE) &&
					TEM_REG_LOCKER.find(reg) == TEM_REG_LOCKER.end() && TEM_REG_LOCKER.find(operant1) == TEM_REG_LOCKER.end())
				{
					// std::cout << "**********everything's ready **********" << std::endl;
					// lock_storing_address(INS);
					++LS_FU_USED;
					RESULT[INS.inQ - 1][EXE] = CYCLE;
					INS.state = EXE;
					std::cout << "Instruction " << INS.num << " begin execute" << std::endl;
					return TRUE;
				}
				else
				{
					return FALSE;
				}
			}
			return FALSE;
		}
		else if ((op.find("Bne") == 0 || op.find("Beq") == 0))
		{
			if (INTEGER_FU_USED < CONS_MAP[INTEGER_ADDER][NUM_FUS])
			{
				std::cout << "debugging here ********3 " << op << "	" << operant1 << "	"  << operant2 << std::endl;
				if (value_available_in_vj_or_qj(INS) == TRUE && TEM_REG_LOCKER.find(operant1) == TEM_REG_LOCKER.end() &&
						value_available_in_vk_or_qk(INS) == TRUE && TEM_REG_LOCKER.find(operant2) == TEM_REG_LOCKER.end())
				{
					++INTEGER_FU_USED;
					RESULT[INS.inQ - 1][EXE] = CYCLE;
					INS.state = EXE;
					return TRUE;
				}
				else
				{
					std::cout << "*******&&&&&&^^^^^^ value not available" << std::endl;
					return FALSE;
				}
			}
			return FALSE;
		}
		else
		{
			std::cout << "debugging here ********4 " << op << "	" << operant1 << "	"  << operant2 << std::endl;
			std::string operant3 = elems[3];
			// for operant1 : not renamed in RAT
			// for operant2 : value known in ARF and not renamed in RAT
			// for operant3 : value known in ARF and not renamed in RAT
			/* three cases */


			if (op.find("Mult.d") == 0)
			{
				std::cout << "debugging here ********7 " << op << "	" << operant1 << "	"  << operant2 << std::endl;
				std::cout << FP_MULT_FU_USED << std::endl;
				if (FP_MULT_FU_USED < CONS_MAP[FP_MULTIPLIER][NUM_FUS])
				{
					std::cout << "debugging here ********7 " << op << "	" << operant1 << "	"  << operant2 << std::endl;
					// if ((FP_MULT_RS[INSTRS[num - 1]->in_rs]->ROB_ENTRY).compare(ROB[RAT[operant1] - 1]->rob_name) == 0 &&
					// 	value_available_in_vj_or_qj(INSTRS, num) == TRUE && TEM_REG_LOCKER.find(operant2) == TEM_REG_LOCKER.end() &&
					// 	value_available_in_vk_or_qk(INSTRS, num) == TRUE && TEM_REG_LOCKER.find(operant3) == TEM_REG_LOCKER.end())
					if (value_available_in_vj_or_qj(INS) == TRUE && TEM_REG_LOCKER.find(operant2) == TEM_REG_LOCKER.end() &&
						value_available_in_vk_or_qk(INS) == TRUE && TEM_REG_LOCKER.find(operant3) == TEM_REG_LOCKER.end())
					{
						// std::cout << "debuging here *********************22222" << std::endl;
						++FP_MULT_FU_USED;
						RESULT[INS.inQ - 1][EXE] = CYCLE;
						INS.state = EXE;
						return TRUE;
					}
					else
					{
						return FALSE;
					}
				}
				return FALSE;
			}
			else if ((op.find("Add.d") == 0 || op.find("Sub.d") == 0))
			{
				if (FP_ADDER_FU_USED < CONS_MAP[FP_ADDER][NUM_FUS])
				{
					// if ((FP_ADDER_RS[INSTRS[num - 1]->in_rs]->ROB_ENTRY).compare(ROB[RAT[operant1] - 1]->rob_name) == 0 &&
					// 	value_available_in_vj_or_qj(INSTRS, num) == TRUE && TEM_REG_LOCKER.find(operant2) == TEM_REG_LOCKER.end() &&
					// 	value_available_in_vk_or_qk(INSTRS, num) == TRUE && TEM_REG_LOCKER.find(operant3) == TEM_REG_LOCKER.end())
					if (value_available_in_vj_or_qj(INS) == TRUE && TEM_REG_LOCKER.find(operant2) == TEM_REG_LOCKER.end() &&
						value_available_in_vk_or_qk(INS) == TRUE && TEM_REG_LOCKER.find(operant3) == TEM_REG_LOCKER.end())
					{
						// std::cout << "debuging here *********************22222" << std::endl;
						++FP_ADDER_FU_USED;
						RESULT[INS.inQ - 1][EXE] = CYCLE;
						INS.state = EXE;
						return TRUE;
					}
					else
					{
						return FALSE;
					}
				}
				return FALSE;
			}
			else if (op.find("Addi") == 0)
			{
				if (INTEGER_FU_USED < CONS_MAP[INTEGER_ADDER][NUM_FUS])
				{
					// if ((INTEGER_ADDER_RS[INSTRS[num - 1]->in_rs]->ROB_ENTRY).compare(ROB[RAT[operant1] - 1]->rob_name) == 0 &&
					// 	value_available_in_vj_or_qj(INSTRS, num) == TRUE && TEM_REG_LOCKER.find(operant2) == TEM_REG_LOCKER.end())
					if (value_available_in_vj_or_qj(INS) == TRUE && TEM_REG_LOCKER.find(operant2) == TEM_REG_LOCKER.end())
					{
						std::cout << "debugging here ********5 " << op << "	" << operant1 << "	"  << operant2 << std::endl;
						++INTEGER_FU_USED;
						RESULT[INS.inQ - 1][EXE] = CYCLE;
						INS.state = EXE;
						return TRUE;
					}
					else
					{
						std::cout << "debugging here ********6 " << op << "	" << operant1 << "	"  << operant2 << std::endl;
						return FALSE;
					}
				}
				return FALSE;
			}
			else
			{

				if (INTEGER_FU_USED < CONS_MAP[INTEGER_ADDER][NUM_FUS])
				{
					// std::cout << (INTEGER_ADDER_RS[INSTRS[num - 1]->in_rs]->ROB_ENTRY).compare(ROB[RAT[operant1] - 1]->rob_name) <<
					// "	" << value_available_in_vj_or_qj(INSTRS, num) << "	" << value_available_in_vk_or_qk(INSTRS, num) << std::endl;
					// if ((INTEGER_ADDER_RS[INSTRS[num - 1]->in_rs]->ROB_ENTRY).compare(ROB[RAT[operant1] - 1]->rob_name) == 0 &&
					// value_available_in_vj_or_qj(INSTRS, num) == TRUE && TEM_REG_LOCKER.find(operant2) == TEM_REG_LOCKER.end() &&
					// value_available_in_vk_or_qk(INSTRS, num) == TRUE && TEM_REG_LOCKER.find(operant3) == TEM_REG_LOCKER.end())
					if (value_available_in_vj_or_qj(INS) == TRUE && TEM_REG_LOCKER.find(operant2) == TEM_REG_LOCKER.end() &&
						value_available_in_vk_or_qk(INS) == TRUE && TEM_REG_LOCKER.find(operant3) == TEM_REG_LOCKER.end())
					{
						// std::cout << "debuging here *********************22222" << std::endl;
						std::cout << "op is: " << op << "	" << "INTEGER_FU_USED is: " << INTEGER_FU_USED << std::endl;
						++INTEGER_FU_USED;
						RESULT[INS.inQ - 1][EXE] = CYCLE;
						INS.state = EXE;
						return TRUE;
					}
					else
					{
						return FALSE;
					}
				}
				return FALSE;
			}
		}
		return FALSE;
	}
}

void move_to_pipeline_do_nothing(std::vector<struct instr>& INS_QUEUE, int num)
{
	if (branch_stall == TRUE)
	{
		std::cout << "Branch stall, wait to solve the branch" << std::endl;
		return;
	}
	if (num > INS_NUM)
	{
		std::cout << "No Instruction to be issued" << std::endl;
		return;
	}
	if (RESULT[SHOULD_FETCH - 1][ISSUE] != 0)
	{
		std::cout << "Instruction " << num << " has issued" << std::endl;
		return;
	}
	std::cout << "Instruction " << TO_PUSH_INTO_QUEUE << " move into queue" << std::endl;
	RESULT[SHOULD_FETCH - 1][ISSUE] = CYCLE;
	++SHOULD_FETCH;

	struct instr tmp;
	tmp._ins = INSTRS[num - 1]._ins;
	tmp.ex_begin = 0;
	tmp.mem_begin = 0;
	tmp.ins_type = INSTRS[num - 1].ins_type;
	tmp.cycle_need = INSTRS[num - 1].cycle_need;
	tmp.has_committed = FALSE;
	tmp.num = INSTRS[num - 1].num;
	tmp.in_rs = EMPTY;
	tmp.state = -1;
	tmp.inQ = INS_QUEUE.size() + 1;
	INS_QUEUE.push_back(tmp);
	// print_instr(INS_QUEUE[INS_QUEUE.size() - 1]);
	++TO_PUSH_INTO_QUEUE;
	// std::cout << RESULT[5][0] << "*************^^^^^^^^^^^^^^" << std::endl;
}

int memory(struct instr& INS)
{
	std::string ins = INS._ins;
	if (ins.find("Ld") == 0 || ins.find("Sd") == 0)
	{
		if (RESULT[INS.inQ - 1][EXE] != 0)
		{
			std::cout << "Instruction " << INS.num << " memory begin" << std::endl;
			RESULT[INS.inQ - 1][MEMORY] = CYCLE;
			INS.state = MEMORY;
			--LS_FU_USED;
			return TRUE;
		}
		else
		{
			std::cout << "Instruction " << INS.num << " not execute yet." << std::endl;
			return FALSE;
		}
	}
	else
	{
		if (RESULT[INS.inQ - 1][EXE] != 0)
		{
			int need = INS.cycle_need;
			int exe_begin = RESULT[INS.inQ - 1][EXE];
			if (CYCLE - exe_begin == need)
			{
				RESULT[INS.inQ - 1][MEMORY] = RESULT[INS.inQ - 1][EXE];
				std::cout << "Instruction " << INS.num << " skip memory" << std::endl;
				INS.state = MEMORY;
				write_back(INS);
				return TRUE;
			}
			else
			{
				std::cout << "Instruction " << INS.num << " not finish executing" << std::endl;
			return FALSE;
			}
		}
		else
		{
			std::cout << "Instruction " << INS.num << " not execute yet." << std::endl;
			return FALSE;
		}
	}
}

// void lock_storing_address(struct instr& INS)
// {
// 	int address = cal_addr(INS);
// 	std::pair<int, int> the_pair (address, TRUE);
// 	MEMORY_LOCK.insert(the_pair);
// }

int write_back(struct instr& INS)
{
	std::string ins = INS._ins;
	std::vector<std::string> elems;
	pro_instr(ins, elems);
	std::string op = elems[0];
	std::string operant1 = elems[1];
	int num = INS.num;
	if (ins.find("Ld") == 0)
	{
	    if (RESULT[INS.inQ - 1][MEMORY] != 0)
	    {
	        int need = INS.cycle_need;
	        int mem_begin = RESULT[INS.inQ - 1][MEMORY];
	        if (CYCLE - mem_begin == need)
	        {
	            RESULT[INS.inQ - 1][WB] = CYCLE;
	            float res = do_instr_cal(INS);
	            // reset_rat(operant1);
	            clear_rs_entry(INS.in_rs, LOAD_STORE_UNIT);
	            std::cout << "Instruction " << num << " write back" << std::endl;
	            INS.state = WB;
	            /* update ROB */
	            return TRUE;
	        }
	        else
	        {
	            std::cout << "Instruction " << num << " wait to finish memory" << std::endl;
	            return FALSE;
	        }
	    }
	    else
	    {
	        std::cout << "Instruction " << num << " not memory" << std::endl;
	        return FALSE;
	    }
	}
	else if (ins.find("Sd") == 0)
	{
		if (RESULT[INS.inQ - 1][MEMORY] != 0)
		{
			int need = INS.cycle_need;
			int mem_begin = RESULT[INS.inQ - 1][MEMORY];
			if (CYCLE - mem_begin == need)
			{
				RESULT[INS.inQ - 1][WB] = CYCLE;
				// float res = do_instr_cal(INSTRS, num);
				// // reset_rat(operant1);
				// clear_rs_entry(num, LOAD_STORE_UNIT);
				std::cout << "Instruction " << num << " write back" << std::endl;
				INS.state = WB;
				/* update ROB */
				return TRUE;
			}
			else
			{
				std::cout << "Instruction " << num << " wait to finish memory" << std::endl;
				return FALSE;
			}
		}
		else
		{
			std::cout << "Instruction " << num << " not memory" << std::endl;
			return FALSE;
		}
	}
	else if (ins.find("Bne") == 0 || ins.find("Beq") == 0)
	{
		if (RESULT[INS.inQ - 1][MEMORY] == 0)
		{
			std::cout << "Instruction " << num << "not memory" << std::endl;
			return FALSE; 
		}
		else
		{
			RESULT[INS.inQ - 1][WB] = CYCLE;
			--INTEGER_FU_USED;
			std::cout << "Instruction " << num << " write back" << std::endl;
			ROB[atoi(INTEGER_ADDER_RS[INS.in_rs]->ROB_ENTRY.substr(3, 1).c_str()) - 1]->finish = TRUE;
			INS.state = WB;
			clear_rs_entry(INS.in_rs, INTEGER_ADDER);
			return TRUE;
		}
	}
	else
	{
		if (RESULT[INS.inQ - 1][MEMORY] == 0)
		{
			std::cout << "Instruction " << num << "not memory" << std::endl;
			return FALSE;
		}
		else
		{
			RESULT[INS.inQ - 1][WB] = CYCLE;
			float res1 = do_instr_cal(INS);
			if (ins.find("Mult.d") == 0)
			{
				// reset_rat(operant1);
				--FP_MULT_FU_USED;
				clear_rs_entry(INS.in_rs, FP_MULTIPLIER);
			}
			else if (ins.find("Add.d") == 0 || ins.find("Sub.d") == 0)
			{
				// reset_rat(operant1);
				--FP_ADDER_FU_USED;
				clear_rs_entry(INS.in_rs, FP_ADDER);
			}
			else
			{
				// reset_rat(operant1);
				--INTEGER_FU_USED;
				clear_rs_entry(INS.in_rs, INTEGER_ADDER);
			}
			std::cout << "Instruction " << num << " write back" << std::endl;
			INS.state = WB;
			return TRUE;
		}
	}
}

float do_instr_cal(struct instr& INS)
{
	std::vector<std::string> elems;
	pro_instr(INS._ins, elems);
	std::string op = elems[0];
	std::string operant1 = elems[1];
	std::string operant2 = elems[2];
	int num = INS.num;
	int _in_rob = INS.inQ;
	if (op.find("Ld") == 0)
	{
		int address = cal_addr(INS);
		float res = MEM[address];
		update_rob_value(_in_rob, res);
		return res;
	}
	else if (op.find("Sd") == 0)
	{
		int address = cal_addr(INS);
		if (LS_RS[INS.in_rs]->VJ != EMPTY)
		{
			if (MEM.find(address) != MEM.end())
			{
				MEM[address] = LS_RS[INS.in_rs]->VJ;
				JUST_COMMIT_ADDR.push_back(address);
				// print_just_commit_addr();
				return LS_RS[INS.in_rs]->VJ;
			}
			else
			{
				std::pair<int, float> the_pair (address, LS_RS[INS.in_rs]->VJ);
				MEM.insert(the_pair);
				JUST_COMMIT_ADDR.push_back(address);
				// print_just_commit_addr();
				return LS_RS[INS.in_rs]->VJ;
			}
		}
		else
		{
			float res = ROB[atoi(LS_RS[INS.in_rs]->QJ.substr(3, 1).c_str()) - 1]->value;
			if (MEM.find(address) != MEM.end())
			{
				MEM[address] = res;
				JUST_COMMIT_ADDR.push_back(address);
				return res;
			}
			else
			{
				std::pair<int, float> the_pair (address, res);
				MEM.insert(the_pair);
				JUST_COMMIT_ADDR.push_back(address);
				return res;
			}
		}
	}
	else
	{
		float val_j, val_k;
		std::string operant3 = elems[3];
		int _in_rs = INS.in_rs;
		if (op.find("Mult.d") == 0)
		{
			if (FP_MULT_RS[_in_rs]->VJ != EMPTY)
				val_j = FP_MULT_RS[_in_rs]->VJ;
			else
			{
				val_j = ROB[atoi(FP_MULT_RS[_in_rs]->QJ.substr(3, 1).c_str()) - 1]->value;
			}
			if (FP_MULT_RS[_in_rs]->VK != EMPTY)
				val_k = FP_MULT_RS[_in_rs]->VK;
			else
			{
				val_k = ROB[atoi(FP_MULT_RS[_in_rs]->QK.substr(3, 1).c_str()) - 1]->value;
			}
			float res = val_j * val_k;
			update_rob_value(_in_rob, res);
			return res;
		}
		else if (op.find("Add.d") == 0)
		{
			if (FP_ADDER_RS[_in_rs]->VJ != EMPTY)
				val_j = FP_ADDER_RS[_in_rs]->VJ;
			else
			{
				val_j = ROB[atoi(FP_ADDER_RS[_in_rs]->QJ.substr(3, 1).c_str()) - 1]->value;
			}
			if (FP_ADDER_RS[_in_rs]->VK != EMPTY)
				val_k = FP_ADDER_RS[_in_rs]->VK;
			else
			{
				val_k = ROB[atoi(FP_ADDER_RS[_in_rs]->QK.substr(3, 1).c_str()) - 1]->value;
			}
			float res = val_j + val_k;
			update_rob_value(_in_rob, res);
			return res;
		}
		else if (op.find("Sub.d") == 0)
		{
			if (FP_ADDER_RS[_in_rs]->VJ != EMPTY)
				val_j = FP_ADDER_RS[_in_rs]->VJ;
			else
			{
				val_j = ROB[atoi(FP_ADDER_RS[_in_rs]->QJ.substr(3, 1).c_str()) - 1]->value;
			}
			if (FP_ADDER_RS[_in_rs]->VK != EMPTY)
				val_k = FP_ADDER_RS[_in_rs]->VK;
			else
			{
				val_k = ROB[atoi(FP_ADDER_RS[_in_rs]->QK.substr(3, 1).c_str()) - 1]->value;
			}
			float res = val_j - val_k;
			update_rob_value(_in_rob, res);
			return res;
		}
		else if (op.find("Sub") == 0)
		{
			if (INTEGER_ADDER_RS[_in_rs]->VJ != EMPTY)
				val_j = INTEGER_ADDER_RS[_in_rs]->VJ;
			else
			{
				val_j = ROB[atoi(INTEGER_ADDER_RS[_in_rs]->QJ.substr(3, 1).c_str()) - 1]->value;
			}
			if (INTEGER_ADDER_RS[_in_rs]->VK != EMPTY)
				val_k = INTEGER_ADDER_RS[_in_rs]->VK;
			else
			{
				val_k = ROB[atoi(INTEGER_ADDER_RS[_in_rs]->QK.substr(3, 1).c_str()) - 1]->value;
			}
			std::cout << "QJ: " <<  INTEGER_ADDER_RS[_in_rs]->ROB_ENTRY << std::endl;
			std::cout << "val_j: " << val_j << " val_k: " << val_k << std::endl;
			float res = val_j - val_k;
			update_rob_value(_in_rob, res);
			return res;
		}
		else if (op.find("Addi") == 0)
		{
			// std::cout << "***************############### in Addi cal instr" << std::endl;
			if (INTEGER_ADDER_RS[_in_rs]->VJ != EMPTY)
				val_j = INTEGER_ADDER_RS[_in_rs]->VJ;
			else
			{
				val_j = ROB[atoi(INTEGER_ADDER_RS[_in_rs]->QJ.substr(3, 1).c_str()) - 1]->value;
			}
			val_k = INTEGER_ADDER_RS[_in_rs]->VK;
			float res = val_j + val_k;
			update_rob_value(_in_rob, res);
			return res;
		}
		else
		{
			if (INTEGER_ADDER_RS[_in_rs]->VJ != EMPTY)
				val_j = INTEGER_ADDER_RS[_in_rs]->VJ;
			else
			{
				val_j = ROB[atoi(INTEGER_ADDER_RS[_in_rs]->QJ.substr(3, 1).c_str()) - 1]->value;
			}
			if (INTEGER_ADDER_RS[_in_rs]->VK != EMPTY)
				val_k = INTEGER_ADDER_RS[_in_rs]->VK;
			else
			{
				val_k = ROB[atoi(INTEGER_ADDER_RS[_in_rs]->QK.substr(3, 1).c_str()) - 1]->value;
			}
			float res = val_j + val_k;
			update_rob_value(_in_rob, res);
			return res;
		}
	}
}

int branch_resolved(int num)
{
	int ex_begin = RESULT[num - 1][EXE];
	int need = CONS_MAP[INTEGER_ADDER][CYCLE_EX];
	if (CYCLE == (ex_begin + need - 1))
	{
		return TRUE;
	}
	else
		return FALSE;
}

void update_rob_value(int num, float res)
{
	ROB[num - 1]->value = res;
	std::pair<std::string, int> the_pair (ROB[num - 1]->dest, TRUE);
	TEM_REG_LOCKER.insert(the_pair);
	ROB[num - 1]->finish = TRUE;
}

void reset_rat(std::string operant)
{
	RAT.erase(operant);
}

void clear_rs_entry(int num, int type)
{
	if (type == INTEGER_ADDER)
	{
		INTEGER_ADDER_RS[num]->BUSY = FALSE;
		INTEGER_ADDER_RS[num]->OP = "";
		INTEGER_ADDER_RS[num]->ROB_ENTRY = "";
		INTEGER_ADDER_RS[num]->VJ = EMPTY;
		INTEGER_ADDER_RS[num]->VK = EMPTY;
		INTEGER_ADDER_RS[num]->QJ = "";
		INTEGER_ADDER_RS[num]->QK = "";
		INTEGER_ADDER_RS[num]->A = EMPTY;
		--INTEGER_ADDER_RS_USED;
	}
	else if (type == FP_ADDER)
	{
		FP_ADDER_RS[num]->BUSY = FALSE;
		FP_ADDER_RS[num]->OP = "";
		FP_ADDER_RS[num]->ROB_ENTRY = "";
		FP_ADDER_RS[num]->VJ = EMPTY;
		FP_ADDER_RS[num]->VK = EMPTY;
		FP_ADDER_RS[num]->QJ = "";
		FP_ADDER_RS[num]->QK = "";
		FP_ADDER_RS[num]->A = EMPTY;
		--FP_ADDER_RS_USED;
	}
	else if (type == FP_MULTIPLIER)
	{
		FP_MULT_RS[num]->BUSY = FALSE;
		FP_MULT_RS[num]->OP = "";
		FP_MULT_RS[num]->ROB_ENTRY = "";
		FP_MULT_RS[num]->VJ = EMPTY;
		FP_MULT_RS[num]->VK = EMPTY;
		FP_MULT_RS[num]->QJ = "";
		FP_MULT_RS[num]->QK = "";
		FP_MULT_RS[num]->A = EMPTY;
		--FP_MULT_RS_USED;
	}
	else
	{
		LS_RS[num]->BUSY = FALSE;
		LS_RS[num]->OP = "";
		LS_RS[num]->ROB_ENTRY = "";
		LS_RS[num]->VJ = EMPTY;
		LS_RS[num]->VK = EMPTY;
		LS_RS[num]->QJ = "";
		LS_RS[num]->QK = "";
		LS_RS[num]->A = EMPTY;
		--LS_RS_USED;
	}
}

int run_to_state(struct instr& INS)
{
	int num = INS.num;
	int state = INS.state;
	int result;
	if (state == -1)
	{
		// std::cout << "Instruction " << num << " asking for issue" << std::endl;
		result = issue(INS);
		// if (result == TRUE)
		// 	std::cout << "Instruction " << num << " doing issue " << std::endl;
	}
	else if (state == ISSUE)
	{
		std::cout << "Instruction " << num << " asking for execute" << std::endl;
		result = execute(INS);
		if (result == TRUE)
			std::cout << "Instruction " << num << " doing execute " << std::endl;

		// std::cout << INSTRS[num - 1]->_ins.compare("Bne") << std::endl;
		if (INS._ins.find("Bne") == 0 || INS._ins.find("Beq") == 0)
		{
			// std::cout << "here for debug" << std::endl;
			if (branch_resolved(INS.inQ) == TRUE)
			{
				if (cal_branch_addr(INS) != -1)
				{
					std::cout << "dump to the instruction " << cal_branch_addr(INS) << std::endl;
					TO_PUSH_INTO_QUEUE = cal_branch_addr(INS);
					branch_stall = FALSE;
				}
				else
				{
					std::cout << "skip the branch" << std::endl;
					branch_stall = FALSE;
				}
			}
		}

	}
	else if (state == EXE)
	{
		/* for branch */
		if (INS._ins.find("Bne") == 0 || INS._ins.find("Beq") == 0)
		{
			if (branch_resolved(INS.inQ) == TRUE)
			{
				if (cal_branch_addr(INS) != -1)
				{
					std::cout << "dump to the instruction " << cal_branch_addr(INS) << std::endl;
					TO_PUSH_INTO_QUEUE = cal_branch_addr(INS);
				}
				else
				{
					std::cout << "skip the branch" << std::endl;
				}
			}
		}
		// std::cout << "Instruction " << num << " asking for memory" << std::endl;
		result = memory(INS);
		// if (result == TRUE)
			// std::cout << "Instruction " << num << " doing memory " << std::endl;
	}
	else if  (state == MEMORY)
	{
		// std::cout << "Instruction " << num << " asking for memory" << std::endl;
		result = write_back(INS);
		// if (result == TRUE)
			// std::cout << "Instruction " << num << " doing write back " << std::endl;
	}
	else if (state == WB)
	{
		std::cout << "Instruction " << INS.num << " asking for commit" << std::endl;
		result = commit(INS);
		// if (result == TRUE)
		// 	std::cout << "Instruction " << INS.num << " finish committing " << std::endl;
	}
	else 
	{
		std::cout << "Instruction " << INS.num << " has committed" << std::endl;
	}
	return result;
}

int commit(struct instr& INS)
{
	if (INS.has_committed == TRUE)
		return TRUE;
	else
	{
		if ((INS._ins).find("Sd") == 0)
		{
			// float res = do_instr_cal(INSTRS, num);
				// // reset_rat(operant1);
				// clear_rs_entry(num, LOAD_STORE_UNIT);
			// int address = cal_addr(INS);
			std::vector<std::string> elems;
			pro_instr(INS._ins, elems);
			/**
			 *  busy, op, vj, vk, qj, qk, a, rat, arf
			 */
			std::string operant2 = elems[2];
			MEMORY_LOCK.erase(operant2);

			if (RESULT[INS.inQ - 1][WB] != 0 && INS.inQ == CAN_COMMIT && commit_is_lock == FALSE)
			{
				float res = do_instr_cal(INS);
				// // reset_rat(operant1);

				ROB[atoi(LS_RS[INS.in_rs]->ROB_ENTRY.substr(3, 1).c_str())- 1]->finish = TRUE;
				clear_rs_entry(INS.in_rs, LOAD_STORE_UNIT);
				commit_is_lock = TRUE;
				RESULT[INS.inQ - 1][COMMIT] = CYCLE;
				// std::pair<int, int> the_pair (INS.num, TRUE);
				// HAS_COMMIT.insert(the_pair);
				// HAS_COMMIT[num] = TRUE;
				INS.has_committed = TRUE;
				INS.state = COMMIT;
				++CAN_COMMIT;
				std::cout << "Instruction " << INS.num << " commit" << std::endl;
				return TRUE;
			}
		}
		if ((INS._ins).find("Bne") == 0 || (INS._ins).find("Beq") == 0)
		{
			if (INS.inQ == CAN_COMMIT && commit_is_lock == FALSE)
			{
				commit_is_lock = TRUE;
				RESULT[INS.inQ - 1][COMMIT] = CYCLE;
				// std::pair<int, int> the_pair (num, TRUE);
				// HAS_COMMIT.insert(the_pair);
				// INS.has_committed = TRUE;
				++CAN_COMMIT;
				std::cout << "Instruction " << INS.num << " commit" << std::endl;
				INS.state = COMMIT;
				INS.has_committed = TRUE;
				return TRUE;
			}	
		}
		if (RESULT[INS.inQ - 1][WB] != 0 && INS.inQ == CAN_COMMIT && commit_is_lock == FALSE)
		{
			// if (ROB[_in_rob]->finish == TRUE)
			// {
				commit_is_lock = TRUE;
				RESULT[INS.inQ - 1][COMMIT] = CYCLE;
				// std::pair<int, int> the_pair (num, TRUE);
				// HAS_COMMIT.insert(the_pair);
				// HAS_COMMIT[num] = TRUE;
				INS.has_committed = TRUE;
				INS.state = COMMIT;
				++CAN_COMMIT;
				std::cout << "Instruction " << INS.num << " commit" << std::endl;
				return TRUE;
			// }
		}
		return FALSE;
	}
}

void update_rat_rob(std::string operant)
{
	if (operant.find("Bne") == 0 || operant.find("Beq") == 0)
	{
		ROB[ROB_NOW_NUM - 1]->dest = operant;
		++ROB_NOW_NUM;
		return;
	}
	ROB[ROB_NOW_NUM - 1]->dest = operant;
	if (RAT.find(operant) == RAT.end())
	{
		std::pair<std::string, int> the_pair (operant, ROB_NOW_NUM);
		RAT.insert(the_pair);
		// std::cout << operant << " " << ROB_NOW_NUM << " ***************** for debugging **************" << RAT[operant] << std::endl << std::endl;
		++ROB_NOW_NUM;
	}
	else
	{
		RAT[operant] = ROB_NOW_NUM;
		++ROB_NOW_NUM;
	}

}

void print_result()
{
	std::cout << "******************************* RESULT ********************************" << std::endl;
	std::cout << "Instruction" << "			" << "I" << "	" << "E" << "	" << "M" << "	" << "W" << "	" << "C" << std::endl;
	std::cout << "-----------------------------------------------------------------------" << std::endl;
	for (int i = 0; i < INS_QUEUE.size(); ++i)
	{
		std::cout << "(" << INS_QUEUE[i].num << ") " << INS_QUEUE[i]._ins << "	|	";
		for (int j = 0; j < 5; j++)
		{
			std::cout << RESULT[i][j] << "	";
		}
		std::cout << std::endl;
	}
	std::cout << "***********************************************************************" << std::endl;
	std::cout << std::endl;
}

void print_rob()
{
	std::cout << "********** ROB **********" << std::endl;
	for (int i = 0; i < 10; ++i)
	{
		std::cout << ROB[i]->rob_name << "	" << ROB[i]->dest << "	" << ROB[i]->value << "	" << ROB[i]->finish << std::endl;
	}
	std::cout << "*************************" << std::endl;
	std::cout << std::endl;
}

void refresh_value()
{
	for (int i = 0 ; i < ROB_SIZE; ++i)
	{
		if (ROB[i]->finish == TRUE && ROB[i]->dest != "" && 
			ROB[i]->finish == TRUE && ROB[i]->dest != "Bne" &&
			ROB[i]->finish == TRUE && ROB[i]->dest != "Beq")
		{
			if (ARF.find(ROB[i]->dest) != ARF.end())  /* need to update value in ARF */
			{
				ARF[ROB[i]->dest] = ROB[i]->value;
			}
			else
			{
				std::pair<std::string, float> the_pair (ROB[i]->dest, ROB[i]->value);
				ARF.insert(the_pair);
			}
		}
	}
}

void print_cons_map()
{
	std::cout << std::endl;
	std::cout << "******** CONS_MAP ********" << std::endl;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			std::cout << CONS_MAP[i][j] << "	";
		}
		std::cout << std::endl;
	}
	std::cout << "**************************" << std::endl;
	std::cout << std::endl;
}

void release_memory_locker()
{
	JUST_COMMIT_ADDR.clear();
}

void print_memory_lock()
{
	std::cout << "************ MEM LOCK ***********" << std::endl;
	for (std::unordered_map<std::string, int>::iterator it = MEMORY_LOCK.begin(); it != MEMORY_LOCK.end(); ++it)
	{
		std::cout << it->first << "	" << it->second << std::endl;
	}
	std::cout << std::endl;
}

void print_just_commit_addr()
{
	std::cout << "************ JUST LOCK ***********" << std::endl;
	for (std::vector<int>::iterator it = JUST_COMMIT_ADDR.begin(); it != JUST_COMMIT_ADDR.end(); ++it)
	{
		std::cout << *it << std::endl;
	}
	std::cout << std::endl;
}

void print_tem_reg_locker()
{
	std::cout << "************ TEM REG ***********" << std::endl;
	for (std::unordered_map<std::string, int>::iterator it = TEM_REG_LOCKER.begin(); 
			it != TEM_REG_LOCKER.end(); ++it)
	{
		std::cout << it->first << "	" << it->second << std::endl;
	}
	std::cout << "********************************" << std::endl;
	std::cout << std::endl;
}

void run_simulator()
{
	init_all();
	init_instructions();
	int loop;
	int result;
	int tmp_should_fetch;
	for (;; ++CYCLE)
	{
		empty_reg_locker();
		release_memory_locker();
		commit_is_lock = FALSE;
		std::cout << std::endl;
		tmp_should_fetch = SHOULD_FETCH;
		std::cout << "************** in cycle " << CYCLE << " ************** " << SHOULD_FETCH << "	" << INS_NUM << std::endl;
		move_to_pipeline_do_nothing(INS_QUEUE, TO_PUSH_INTO_QUEUE);

		for (loop = 0; loop < INS_QUEUE.size(); ++loop)
		{
			result = run_to_state(INS_QUEUE[loop]);
			// print_instr(INS_QUEUE[INS_QUEUE.size() - 1]);
		}
		// if (CYCLE == 51)
		if (INS_QUEUE[INS_QUEUE.size() - 1].has_committed == TRUE)
		{
			// std::cout << "debugging********* " << INS_QUEUE[INS_QUEUE.size() - 1].has_committed << std::endl;
			// print_instr(INS_QUEUE[INS_QUEUE.size() - 1]);
			refresh_value();
			print_rat();
			print_rob();
			print_memory();
			print_rs();
			print_arf();
			print_result();
			print_memory_lock();
			print_just_commit_addr();
			
			print_static_value();
			print_tem_reg_locker();
			
			// print_instr(INS_QUEUE[1]);
			break;
		}
	}

	std::cout << std::endl;
	std::cout << std::endl;
}

void empty_reg_locker()
{
	TEM_REG_LOCKER.clear();
}

int cal_branch_addr(struct instr& INS)
{
	int _in_rs = INS.in_rs;
	float val_j, val_k;
	int offset = INTEGER_ADDER_RS[_in_rs]->A;
	int res;  /* if do it, return address, else return -1 */
	std::string ins = INS._ins;
	if (INTEGER_ADDER_RS[_in_rs]->VJ != EMPTY)
		val_j = INTEGER_ADDER_RS[_in_rs]->VJ;
	else
		val_j = ROB[atoi(INTEGER_ADDER_RS[_in_rs]->QJ.substr(3, 1).c_str()) - 1]->value;
	if (INTEGER_ADDER_RS[_in_rs]->VK != EMPTY)
		val_k = INTEGER_ADDER_RS[_in_rs]->VK;
	else
		val_k = ROB[atoi(INTEGER_ADDER_RS[_in_rs]->QK.substr(3, 1).c_str()) - 1]->value;
	if (ins.find("Beq") == 0)
	{
		if (val_j == val_k)
			res = INS.num + (4 + offset * 4) / 4;
		else 
			res = -1;
	}
	else 
	{
		if (val_j != val_k)
			res = INS.num + (4 + offset * 4) / 4;
		else
			res = -1;
	}
	return res;
}

void print_instr(struct instr INS)
{
	std::cout << std::endl;
	std::cout << "************* instruction ***********" << std::endl;
	std::cout << "_ins: " << "	" << INS._ins << std::endl;
	std::cout << "ex_begin: " << "	" << INS.ex_begin << std::endl;
	std::cout << "mem_begin: " << "	" << INS.mem_begin << std::endl;
	std::cout << "ins_type: " << "	" << INS.ins_type << std::endl;
	std::cout << "cycle_need: " << "	" << INS.cycle_need << std::endl;
	std::cout << "has_committed: " << "	" << INS.has_committed << std::endl;
	std::cout << "num: " << "	" << INS.num << std::endl;
	std::cout << "in_rs: " << "	" << INS.in_rs << std::endl;
	std::cout << "state: " << "	" << INS.state << std::endl;
	std::cout << "inQ: " << "	" << INS.inQ << std::endl;
	std::cout << "**************************************" << std::endl;
	std::cout << std::endl;
}

void print_static_value()
{
	std::cout << std::endl;
	std::cout << "************* Static Value************" << std::endl;
	std::cout << "INSTRUCTION IN QUEUE: " << "	" << INS_QUEUE.size() << std::endl;
	std::cout << "INS_NUM: " << "	" << INS_NUM << std::endl;
	std::cout << "ROB_NOW_NUM: " << "	" << ROB_NOW_NUM << std::endl;
	std::cout << "SHOULD_FETCH: " << "	" << SHOULD_FETCH << std::endl;
	std::cout << "CAN_COMMIT: " << "	" << CAN_COMMIT << std::endl;
	std::cout << "TO_PUSH_INTO_QUEUE: " << "	" << TO_PUSH_INTO_QUEUE << std::endl;
	std::cout << "branch stall: " << "	" << branch_stall << std::endl;
	std::cout << "**************************************" << std::endl;
	std::cout << std::endl;
}

int main(int argc, char** argv)
{
	read_file("test.txt");
	print_cons_map();
	run_simulator();
	return 0;
}
































