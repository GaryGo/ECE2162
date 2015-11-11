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
	RESULT = new int* [INS_NUM];
	for (i = 0; i < INS_NUM; ++i)
	{
		RESULT[i] = new int[5];
	}

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

int fetch_ins(const std::string ins, int order)
{
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
				INSTRS[order - 1]->in_rs = i;
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
				INSTRS[order - 1]->in_rs = i;
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

	}
	else
	{
		std::string operant3 = elems[3];
		/* three situation.  (Add, Addi, Sub), (Add.d, Sub.d), (Mul.d) */
		if (op == "Add" || op == "Addi" || op == "Sub")
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
					INSTRS[order - 1]->in_rs = i;
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
					INSTRS[order - 1]->in_rs = i;
					has_fetch = TRUE;
					++FP_ADDER_RS_USED;
					break;
				}
			}
		}
		else
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
					INSTRS[order - 1]->in_rs = i;
					has_fetch = TRUE;
					++FP_MULT_RS_USED;
					break;
				}
			}
		}
	}
	return has_fetch;
}

int cal_addr(struct instr **INSTRS, int num)
{
	int res;
	if (LS_RS[INSTRS[num - 1]->in_rs]->A != EMPTY)
	{
		res = LS_RS[INSTRS[num - 1]->in_rs]->A;
		return res;
	}
	else 
	{
		res = int(ROB[atoi(LS_RS[INSTRS[num - 1]->in_rs]->QK.substr(3, 1).c_str()) - 1]->value + LS_RS[INSTRS[num - 1]->in_rs]->VK);
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


int addr_ready(struct instr **INSTRS, int num)
{
	int loc1 = INSTRS[num - 1]->_ins.find("(");
	int loc2 = INSTRS[num - 1]->_ins.find(")");
	std::string reg = INSTRS[num - 1]->_ins.substr(loc1 + 1, loc2 - loc1 - 1);
	if (LS_RS[INSTRS[num - 1]->in_rs]->A != EMPTY)
	{
		if (MEMORY_LOCK.find(LS_RS[INSTRS[num - 1]->in_rs]->A) == MEMORY_LOCK.end() && 
			store_memory_just_commit(LS_RS[INSTRS[num - 1]->in_rs]->A) == FALSE)
			return TRUE;
		else 
			return FALSE;
	}
	else if (ROB[atoi(LS_RS[INSTRS[num - 1]->in_rs]->QK.substr(3, 1).c_str()) - 1]->finish == TRUE && TEM_REG_LOCKER.find(reg) == TEM_REG_LOCKER.end())
	{
		int address = int(ROB[atoi(LS_RS[INSTRS[num - 1]->in_rs]->QK.substr(3, 1).c_str()) - 1]->value + LS_RS[INSTRS[num - 1]->in_rs]->VK);
		if (MEMORY_LOCK.find(address) == MEMORY_LOCK.end() &&
			store_memory_just_commit(address) == FALSE)
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
		if (ROB[RAT[operant] - 1]->finish == TRUE)
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
	INSTRS = new struct instr *[INS_NUM];
	for (i = 0; i < INS_NUM; ++i)
	{
		INSTRS[i] = new struct instr;
		std::string the_ins = ALL_INS[i + 1];
		// std::cout << "the ins is: " << the_ins << std::endl;
		INSTRS[i]->_ins = the_ins;
		INSTRS[i]->num = i + 1;
		INSTRS[i]->in_rs = EMPTY;
		INSTRS[i]->state = -1;
		if (the_ins.find("Ld") == 0 || the_ins.find("Sd") == 0)
		{
			// std::cout << "in Ld, Sd" << std::endl;
			INSTRS[i]->ins_type = LOAD_STORE_UNIT;
			INSTRS[i]->cycle_need = CONS_MAP[LOAD_STORE_UNIT][CYCLE_MEM];
			INSTRS[i]->has_committed = FALSE;
		}
		else if (the_ins.find("Add.d") == 0 || the_ins.find("Sub.d") == 0)
		{
			// std::cout << "in Add.d, Sub.d" << std::endl;
			INSTRS[i]->ins_type = FP_ADDER;
			INSTRS[i]->cycle_need = CONS_MAP[FP_ADDER][CYCLE_EX];
			INSTRS[i]->has_committed = FALSE;
		}
		else if (the_ins.find("Mult.d") == 0)
		{
			// std::cout << "in mult.d" << std::endl;
			INSTRS[i]->ins_type = FP_MULTIPLIER;
			INSTRS[i]->cycle_need = CONS_MAP[FP_MULTIPLIER][CYCLE_EX];
			INSTRS[i]->has_committed = FALSE;
		}
		else
		{
			// std::cout << "in other" << std::endl;
			INSTRS[i]->ins_type = INTEGER_ADDER;
			// std::cout << "type is: " << INTEGER_ADDER << std::endl;
			INSTRS[i]->cycle_need = CONS_MAP[INTEGER_ADDER][CYCLE_EX];
			INSTRS[i]->has_committed = FALSE;
			// std::cout << "cycle needed is: " << CONS_MAP[INTEGER_ADDER][CYCLE_EX] << std::endl;
		}
	}
}

int check_rs_available(struct instr **INSTRS, int num)
{
	// std::cout << INSTRS[num]->ins_type << std::endl;
	// std::cout << INSTRS[num]->_ins << std::endl;
	--num;
	if (INSTRS[num]->ins_type == INTEGER_ADDER)
	{
		if (INTEGER_ADDER_RS_USED == CONS_MAP[INTEGER_ADDER][NUM_RS])
			return FALSE;
		else
			return TRUE;
	}
	else if (INSTRS[num]->ins_type == FP_ADDER)
	{
		if (FP_ADDER_RS_USED == CONS_MAP[FP_ADDER][NUM_RS])
			return FALSE;
		else
			return TRUE;
	}
	else if (INSTRS[num]->ins_type == FP_MULTIPLIER)
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

int issue(struct instr **INSTRS, int num)
{
	std::cout << "Instruction " << num << " state is: " << INSTRS[num - 1]->state << std::endl;
	if (INSTRS[num - 1]->state == -1)
	{
		int tmp = check_rs_available(INSTRS, num);
		if (tmp == TRUE) /* entry available for this instruction */
		{
			/* remember to minus one */
			// RESULT[SHOULD_FETCH - 1][ISSUE] = CYCLE;
			// std::cout << RESULT[SHOULD_FETCH - 1][ISSUE] << std::endl;
			// std::cout << INSTRS[num - 1]->_ins << std::endl;
			// std::cout << renamed_in_rat(first_operant(INSTRS[num - 1]->_ins)) << std::endl;
				// std::cout << "************** message for debugging ************** "<< RAT[first_operant(INSTRS[num - 1]->_ins)] << std::endl;
				// std::cout << "here" << std::endl;
			int has_fetch = fetch_ins(INSTRS[num - 1]->_ins, num);
			// std::cout << "here: " << has_fetch << std::endl;
			std::pair<int, int> the_pair (num, has_fetch);
			HAS_FETCH.insert(the_pair);
			// ++SHOULD_FETCH;
			std::cout << "Instruction " << num << " issue" << std::endl;
			INSTRS[num - 1]->state = ISSUE;
			return TRUE;

		}
		std::cout << num << " RS is full." << std::endl;
		return FALSE;
	}
	return TRUE;
}

int value_available_in_vk_or_qk(struct instr **INSTRS, int num)
{
	std::string ins = INSTRS[num - 1]->_ins;
	int _in_rs = INSTRS[num - 1]->in_rs;
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

int value_available_in_vj_or_qj(struct instr **INSTRS, int num)
{
	std::string ins = INSTRS[num - 1]->_ins;
	int _in_rs = INSTRS[num - 1]->in_rs;
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

int execute(struct instr **INSTRS, int num)
{

	if (RESULT[num - 1][ISSUE] == 0)
	{
		std::cout << "Instruction " << num << " did not issue" << std::endl;
		return FALSE;
	}
	else
	{
		// std::cout << "debuging here ********************* 11111" << std::endl;
		std::vector<std::string> elems;
		pro_instr(INSTRS[num - 1]->_ins, elems);
		std::string op = elems[0];
		std::string operant1 = elems[1];
		std::string operant2 = elems[2];
		// std::cout << op << "	" << operant1 << "	" << operant2 << std::endl;
		if (op.compare("Ld") == 0)
		{
			int _in_rs = INSTRS[num - 1]->in_rs;
			// std::cout << _in_rs << " in_rs" << std::endl;
			if (LS_RS[INSTRS[num - 1]->in_rs]->ROB_ENTRY.compare(ROB[RAT[operant1] - 1]->rob_name) != 0)
			{
				std::cout << "RAT for " << operant1 << " in used" << std::endl;
				return FALSE;
			}
			else
			{
				// std::cout << op << "	" << operant1 << "	" << operant2 << std::endl;
				
				// std::cout << "*****************^^^^^^^^********************" << std::endl;
				if (addr_ready(INSTRS, num) == TRUE)
				{
					std::cout << "Instruction " << num << " begin execute" << std::endl;
						
					RESULT[num - 1][EXE] = CYCLE;
					INSTRS[num - 1]->state = EXE;
					return TRUE;
				}
				else
				{
					std::cout << "Instruction " << num << " Address not ready" << std::endl;
					return FALSE;
				}			
			}
		}
		else if (op.compare("Sd") == 0)
		{
			int _in_rs = INSTRS[num - 1]->in_rs;
			std::cout << _in_rs << " in_rs" << std::endl;
			// std::cout << op << "	" << operant1 << "	" << operant2 << std::endl;
			std::cout << "Instruction " << num << " begin execute" << std::endl;
			// std::cout << "*****************^^^^^^^^********************" << std::endl;
			int loc1 = operant2.find("(");
			int loc2 = operant2.find(")");
			std::string reg = operant2.substr(loc1 + 1, loc2 - loc1 - 1);
			if (addr_ready(INSTRS, num) == TRUE && (LS_RS[INSTRS[num - 1]->in_rs]->VJ != EMPTY || 
				ROB[atoi(LS_RS[INSTRS[num - 1]->in_rs]->QJ.substr(3, 1).c_str()) - 1]->finish == TRUE) && 
				TEM_REG_LOCKER.find(reg) == TEM_REG_LOCKER.end() && TEM_REG_LOCKER.find(operant1) == TEM_REG_LOCKER.end())
			{
				// std::cout << "**********everything's ready **********" << std::endl;
				lock_storing_address(INSTRS, num);
				RESULT[num - 1][EXE] = CYCLE;
				INSTRS[num - 1]->state = EXE;
				return TRUE;
			}
			else 
			{
				return FALSE;
			}
		}
		else
		{
			std::string operant3 = elems[3];
			// for operant1 : not renamed in RAT
			// for operant2 : value known in ARF and not renamed in RAT
			// for operant3 : value known in ARF and not renamed in RAT
			/* three cases */


			if (op.find("Mult.d") == 0)
			{
				if ((FP_MULT_RS[INSTRS[num - 1]->in_rs]->ROB_ENTRY).compare(ROB[RAT[operant1] - 1]->rob_name) == 0 &&
					value_available_in_vj_or_qj(INSTRS, num) == TRUE && TEM_REG_LOCKER.find(operant2) == TEM_REG_LOCKER.end() &&
					value_available_in_vk_or_qk(INSTRS, num) == TRUE && TEM_REG_LOCKER.find(operant3) == TEM_REG_LOCKER.end())
				{
					// std::cout << "debuging here *********************22222" << std::endl;
					RESULT[num - 1][EXE] = CYCLE;
					INSTRS[num - 1]->state = EXE;
					return TRUE;
				}
				else
				{
					return FALSE;
				}
			}
			else if (op.find("Add.d") == 0 || op.find("Sub.d") == 0)
			{
				if ((FP_ADDER_RS[INSTRS[num - 1]->in_rs]->ROB_ENTRY).compare(ROB[RAT[operant1] - 1]->rob_name) == 0 &&
					value_available_in_vj_or_qj(INSTRS, num) == TRUE && TEM_REG_LOCKER.find(operant2) == TEM_REG_LOCKER.end() &&
					value_available_in_vk_or_qk(INSTRS, num) == TRUE && TEM_REG_LOCKER.find(operant3) == TEM_REG_LOCKER.end())
				{
					// std::cout << "debuging here *********************22222" << std::endl;
					RESULT[num - 1][EXE] = CYCLE;
					INSTRS[num - 1]->state = EXE;
					return TRUE;
				}
				else
				{
					return FALSE;
				}
			}
			else
			{
				if ((INTEGER_ADDER_RS[INSTRS[num - 1]->in_rs]->ROB_ENTRY).compare(ROB[RAT[operant1] - 1]->rob_name) == 0 &&
					value_available_in_vj_or_qj(INSTRS, num) == TRUE && TEM_REG_LOCKER.find(operant2) == TEM_REG_LOCKER.end() &&
					value_available_in_vk_or_qk(INSTRS, num) == TRUE && TEM_REG_LOCKER.find(operant3) == TEM_REG_LOCKER.end())
				{
					// std::cout << "debuging here *********************22222" << std::endl;
					RESULT[num - 1][EXE] = CYCLE;
					INSTRS[num - 1]->state = EXE;
					return TRUE;
				}
				else
				{
					return FALSE;
				}
			}
		}
	}
}

void move_to_pipeline_do_nothing()
{
	std::cout << "Instruction " << SHOULD_FETCH << " move into pipeline" << std::endl;
	if (RESULT[SHOULD_FETCH - 1][ISSUE] != 0)
		return;
	RESULT[SHOULD_FETCH - 1][ISSUE] = CYCLE;
	++SHOULD_FETCH;
	// std::cout << RESULT[5][0] << "*************^^^^^^^^^^^^^^" << std::endl;
}

int memory(struct instr **INSTRS, int num)
{
	std::string ins = INSTRS[num - 1]->_ins;
	if (ins.find("Ld") == 0 || ins.find("Sd") == 0)
	{
		if (RESULT[num - 1][EXE] != 0)
		{
			std::cout << "Instruction " << num << " memory begin" << std::endl;
			RESULT[num - 1][MEMORY] = CYCLE;
			INSTRS[num - 1]->state = MEMORY;
			return TRUE;
		}
		else
		{
			std::cout << "Instruction " << num << " not execute yet." << std::endl;
			return FALSE;
		}
	}
	else
	{
		if (RESULT[num - 1][EXE] != 0)
		{
			int need = INSTRS[num - 1]->cycle_need;
			int exe_begin = RESULT[num - 1][EXE];
			if (CYCLE - exe_begin == need)
			{
				RESULT[num - 1][MEMORY] = RESULT[num - 1][EXE];
				std::cout << "Instruction " << num << " skip memory" << std::endl;
				INSTRS[num - 1]->state = MEMORY;
				write_back(INSTRS, num);
				return TRUE;
			}
			else
			{
				std::cout << "Instruction " << num << " not finish executing" << std::endl;
			return FALSE;
			}
		}
		else
		{
			std::cout << "Instruction " << num << " not execute yet." << std::endl;
			return FALSE;
		}
	}
}

void lock_storing_address(struct instr **INSTRS, int num)
{
	int address = cal_addr(INSTRS, num);
	std::pair<int, int> the_pair (address, TRUE);
	MEMORY_LOCK.insert(the_pair);
}

int write_back(struct instr **INSTRS, int num)
{
	std::string ins = INSTRS[num - 1]->_ins;
	std::vector<std::string> elems;
	pro_instr(ins, elems);
	std::string op = elems[0];
	std::string operant1 = elems[1];
	if (ins.find("Ld") == 0)
	{
		if (RESULT[num - 1][MEMORY] != 0)
		{
			int need = INSTRS[num - 1]->cycle_need;
			int mem_begin = RESULT[num - 1][MEMORY];
			if (CYCLE - mem_begin == need)
			{
				RESULT[num - 1][WB] = CYCLE;
				float res = do_instr_cal(INSTRS, num);
				// reset_rat(operant1);
				clear_rs_entry(num, LOAD_STORE_UNIT);
				std::cout << "Instruction " << num << " write back" << std::endl;
				INSTRS[num - 1]->state = WB;
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
		if (RESULT[num - 1][MEMORY] != 0)
		{
			int need = INSTRS[num - 1]->cycle_need;
			int mem_begin = RESULT[num - 1][MEMORY];
			if (CYCLE - mem_begin == need)
			{
				RESULT[num - 1][WB] = CYCLE;
				// float res = do_instr_cal(INSTRS, num);
				// // reset_rat(operant1);
				// clear_rs_entry(num, LOAD_STORE_UNIT);
				std::cout << "Instruction " << num << " write back" << std::endl;
				INSTRS[num - 1]->state = WB;
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
	else
	{
		if (RESULT[num - 1][MEMORY] == 0)
		{
			std::cout << "Instruction " << num << "not memory" << std::endl;
			return FALSE;
		}
		else
		{
			RESULT[num - 1][WB] = CYCLE;
			float res1 = do_instr_cal(INSTRS, num);
			if (ins.find("Mult.d") == 0)
			{
				// reset_rat(operant1);
				clear_rs_entry(num, FP_MULTIPLIER);
			}
			else if (ins.find("Add.d") == 0 || ins.find("Sub.d") == 0)
			{
				// reset_rat(operant1);
				clear_rs_entry(num, FP_ADDER);
			}
			else
			{
				// reset_rat(operant1);
				clear_rs_entry(num, INTEGER_ADDER);
			}
			std::cout << "Instruction " << num << " write back" << std::endl;
			INSTRS[num - 1]->state = WB;
			return TRUE;
		}
	}
}

float do_instr_cal(struct instr **INSTRS, int num)
{
	std::vector<std::string> elems;
	pro_instr(INSTRS[num - 1]->_ins, elems);
	std::string op = elems[0];
	std::string operant1 = elems[1];
	std::string operant2 = elems[2];
	int _in_rob = num;
	if (op.find("Ld") == 0)
	{
		int address = cal_addr(INSTRS, num);
		float res = MEM[address];
		update_rob_value(_in_rob, res);
		return res;
	}
	else if (op.find("Sd") == 0)
	{
		int address = cal_addr(INSTRS, num);
		if (LS_RS[INSTRS[num - 1]->in_rs]->VJ != EMPTY)
		{
			if (MEM.find(address) != MEM.end())
			{
				MEM[address] = LS_RS[INSTRS[num - 1]->in_rs]->VJ;
				return LS_RS[INSTRS[num - 1]->in_rs]->VJ;
			}
			else
			{
				std::pair<int, float> the_pair (address, LS_RS[INSTRS[num - 1]->in_rs]->VJ);
				MEM.insert(the_pair);
				JUST_COMMIT_ADDR.push_back(address);
				return LS_RS[INSTRS[num - 1]->in_rs]->VJ;
			}
		}
		else 
		{
			float res = ROB[atoi(LS_RS[INSTRS[num - 1]->in_rs]->QJ.substr(3, 1).c_str()) - 1]->value;
			if (MEM.find(address) != MEM.end())
			{
				MEM[address] = res;
				return res;
			}
			else
			{
				std::pair<int, float> the_pair (address, res);
				MEM.insert(the_pair);
				return res;
			}
		}		
	}
	else
	{
		float val_j, val_k;
		std::string operant3 = elems[3];
		int _in_rs = INSTRS[num - 1]->in_rs;
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
	int loc = INSTRS[num - 1]->in_rs;
	if (type == INTEGER_ADDER)
	{
		INTEGER_ADDER_RS[loc]->BUSY = FALSE;
		INTEGER_ADDER_RS[loc]->OP = "";
		INTEGER_ADDER_RS[loc]->ROB_ENTRY = "";
		INTEGER_ADDER_RS[loc]->VJ = EMPTY;
		INTEGER_ADDER_RS[loc]->VK = EMPTY;
		INTEGER_ADDER_RS[loc]->QJ = "";
		INTEGER_ADDER_RS[loc]->QK = "";
		INTEGER_ADDER_RS[loc]->A = EMPTY;
		--INTEGER_ADDER_RS_USED;
	}
	else if (type == FP_ADDER)
	{
		FP_ADDER_RS[loc]->BUSY = FALSE;
		FP_ADDER_RS[loc]->OP = "";
		FP_ADDER_RS[loc]->ROB_ENTRY = "";
		FP_ADDER_RS[loc]->VJ = EMPTY;
		FP_ADDER_RS[loc]->VK = EMPTY;
		FP_ADDER_RS[loc]->QJ = "";
		FP_ADDER_RS[loc]->QK = "";
		FP_ADDER_RS[loc]->A = EMPTY;
		--FP_ADDER_RS_USED;
	}
	else if (type == FP_MULTIPLIER)
	{
		FP_MULT_RS[loc]->BUSY = FALSE;
		FP_MULT_RS[loc]->OP = "";
		FP_MULT_RS[loc]->ROB_ENTRY = "";
		FP_MULT_RS[loc]->VJ = EMPTY;
		FP_MULT_RS[loc]->VK = EMPTY;
		FP_MULT_RS[loc]->QJ = "";
		FP_MULT_RS[loc]->QK = "";
		FP_MULT_RS[loc]->A = EMPTY;
		--FP_MULT_RS_USED;
	}
	else
	{
		LS_RS[loc]->BUSY = FALSE;
		LS_RS[loc]->OP = "";
		LS_RS[loc]->ROB_ENTRY = "";
		LS_RS[loc]->VJ = EMPTY;
		LS_RS[loc]->VK = EMPTY;
		LS_RS[loc]->QJ = "";
		LS_RS[loc]->QK = "";
		LS_RS[loc]->A = EMPTY;
		--LS_RS_USED;
	}
}

int run_to_state(struct instr **INSTRS, int num)
{
	int state = INSTRS[num - 1]->state;
	int result;
	if (state == -1)
	{
		// std::cout << "Instruction " << num << " asking for issue" << std::endl;
		result = issue(INSTRS, num);
		// if (result == TRUE)
		// 	std::cout << "Instruction " << num << " doing issue " << std::endl;
	}
	else if (state == ISSUE)
	{
		std::cout << "Instruction " << num << " asking for execute" << std::endl;
		result = execute(INSTRS, num);
		if (result == TRUE)
			std::cout << "Instruction " << num << " doing execute " << std::endl;
	}
	else if (state == EXE)
	{
		// std::cout << "Instruction " << num << " asking for memory" << std::endl;
		result = memory(INSTRS, num);
		// if (result == TRUE)
			// std::cout << "Instruction " << num << " doing memory " << std::endl;
	}
	else if  (state == MEMORY)
	{
		// std::cout << "Instruction " << num << " asking for memory" << std::endl;
		result = write_back(INSTRS, num);
		// if (result == TRUE)
			// std::cout << "Instruction " << num << " doing write back " << std::endl;
	}
	else
	{
		// std::cout << "Instruction " << num << " asking for commit" << std::endl;
		result = commit(INSTRS, num);
		// if (result == TRUE)
			// std::cout << "Instruction " << num << " doing commit " << std::endl;
	}
	return result;
}

int commit(struct instr **INSTRS, int num)
{
	if (INSTRS[num - 1]->has_committed == TRUE)
		return TRUE;
	else
	{
		if ((INSTRS[num - 1]->_ins).find("Sd") == 0)
		{
			// float res = do_instr_cal(INSTRS, num);
				// // reset_rat(operant1);
				// clear_rs_entry(num, LOAD_STORE_UNIT);
			int address = cal_addr(INSTRS, num);
			MEMORY_LOCK.erase(address);
			if (RESULT[num - 1][WB] != 0 && num == CAN_COMMIT && commit_is_lock == FALSE)
			{
				float res = do_instr_cal(INSTRS, num);
				// // reset_rat(operant1);
				
				ROB[atoi(LS_RS[INSTRS[num - 1]->in_rs]->ROB_ENTRY.substr(3, 1).c_str())- 1]->finish = TRUE;
				clear_rs_entry(num, LOAD_STORE_UNIT);
				commit_is_lock = TRUE;
				RESULT[num - 1][COMMIT] = CYCLE;
				std::pair<int, int> the_pair (num, TRUE);
				HAS_COMMIT.insert(the_pair);
				HAS_COMMIT[num] = TRUE;
				INSTRS[num - 1]->has_committed = TRUE;
				++CAN_COMMIT;
				std::cout << "Instruction " << num << " commit" << std::endl;
				return TRUE;
			}
		}
		if (RESULT[num - 1][WB] != 0 && num == CAN_COMMIT && commit_is_lock == FALSE)
		{
			if (ROB[num - 1]->finish == TRUE)
			{
				commit_is_lock = TRUE;
				RESULT[num - 1][COMMIT] = CYCLE;
				std::pair<int, int> the_pair (num, TRUE);
				HAS_COMMIT.insert(the_pair);
				HAS_COMMIT[num] = TRUE;
				INSTRS[num - 1]->has_committed = TRUE;
				++CAN_COMMIT;
				std::cout << "Instruction " << num << " commit" << std::endl;
				return TRUE;
			}
		}
		return FALSE;
	}
}

void update_rat_rob(std::string operant)
{
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
	std::cout << "************* RESULT **************" << std::endl;
	for (int i = 0; i < INS_NUM; ++i)
	{
		for (int j = 0; j < 5; j++)
		{
			std::cout << RESULT[i][j] << "	";
		}
		std::cout << std::endl;
	}
	std::cout << "***********************************" << std::endl;
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
		if (ROB[i]->finish == TRUE && ROB[i]->dest.compare("") != 0)
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
	for (std::unordered_map<int, int>::iterator it = MEMORY_LOCK.begin(); it != MEMORY_LOCK.end(); ++it)
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
		if (tmp_should_fetch <= INS_NUM)
		{
			move_to_pipeline_do_nothing();
			for (loop = 1; loop <= tmp_should_fetch; ++loop)
			{
				result = run_to_state(INSTRS, loop);
			}
		}
		else
		{
			for (loop = 1; loop <= INS_NUM; ++loop)
			{
				result = run_to_state(INSTRS, loop);		
			}
		}
		if (HAS_COMMIT[INS_NUM] == TRUE)
		{
			// std::cout << "LS_RS_USED is: " << LS_RS_USED << std::endl;
			refresh_value();
			print_rat();
			print_rob();
			print_memory();
			print_rs();
			print_result();
			print_memory_lock();
			print_just_commit_addr();
			print_arf();
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



int main(int argc, char** argv)
{
	read_file("test.txt");
	print_cons_map();
	run_simulator();
	return 0;
}






























