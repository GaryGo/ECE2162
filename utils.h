#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <sstream>
#include <unordered_map>
#include <queue>


#define INTEGER_ADDER 		0   	/* Integer adder */
#define FP_ADDER 			    1 		/* FP adder */
#define FP_MULTIPLIER 		2 		/* FP multiplier */
#define LOAD_STORE_UNIT 	3 		/* Load/store uint */
#define NUM_RS 				    0  		/* # of rs */
#define CYCLE_EX 			    1 		/* Cycles in EX */
#define CYCLE_MEM 			  2 		/* Cycles in Mem */
#define NUM_FUS 			    3 		/* # of FUs */

/* for result talbe */
#define ISSUE 				    0
#define EXE 				      1
#define MEMORY 				    2
#define WB 					      3
#define COMMIT 				    4

#define TRUE 				      1
#define FALSE				      0
#define EMPTY				      1000000.0


// Ld F2, 45(R3)
// Add.i F3, F2, R2
// Mult.d F5, F2, F4


/****************************************************************************************
 *     								           Global Variables                                       *
 ****************************************************************************************/

/**
 * instructions number
 */
static int INS_NUM = 0;

/**
 *  constant map for calculation
 */
int CONS_MAP[4][4];

/**
 *  ROB entries size
 */
int ROB_SIZE;

/**
 *  ROB recent entry number
 */
static int ROB_NOW_NUM = 1;

/**
 *  a pointer to ROB array
 */
struct rob_entry **ROB;

/**
 *  an unorder_map for registers and their value
 */
std::unordered_map<std::string, float> REG;

/**
 *  an unorder_map for Memory address and their value
 */
std::unordered_map<int, float> MEM;

/**
 *  instructions set in order
 */
std::unordered_map<int, std::string> ALL_INS;

/**
 *  cycle number, initiazed to 1
 */
static int CYCLE = 1;

/**
 *  reslut table
 */
static int RESULT[100][5];
/*
	RESULT = new int*[number of instructions];
	for (int i = 0; i < num of ins; ++i)
	{
		RESULT[i] = new int[5];
	}
*/

/**
 *  RAT unordered_map
 */
static std::unordered_map<std::string, int> RAT;

/**
 * Architecture Reg File
 */
static std::unordered_map<std::string, float> ARF;

/**
 *  Integer adder RS
 */
static struct RS **INTEGER_ADDER_RS;

/**
 *  entries used in Integer adder RS
 */
static int INTEGER_ADDER_RS_USED = 0;

/**
 *  FP adder RS
 */
static struct RS **FP_ADDER_RS;

/**
 *  entries used in FP adder RS
 */
static int FP_ADDER_RS_USED = 0;

/**
 *  FP multiplier RS
 */
static struct RS **FP_MULT_RS;

/**
 *  entries used in FP multiplier RS
 */
static int FP_MULT_RS_USED = 0;

/**
 *  Load/store unit RS
 */
static struct RS **LS_RS;

/**
 *  entries used in LS_RS
 */
static int LS_RS_USED = 0;

/**
 *  whether a instruction has been fetched
 */
static std::unordered_map<int, int> HAS_FETCH;

/**
 *  whether a instruction has been committed
 */
static std::unordered_map<int, int> HAS_COMMIT;

// *
//  *  pointer to all instr stucture
 
// static struct instr **INSTRS;

/**
 *  redefine the pointer INSTRS to vector
 */
static std::vector<struct instr> INSTRS;

/**
 *  next number of instruction needed to fetch
 */
static int SHOULD_FETCH = 1;

/**
 *  the instruction number that can commit
 */
static int CAN_COMMIT = 1;

/**
 *  commit lock
 */
int commit_is_lock = FALSE;

/**
 * a tempory MAP that lock the currently commit register for next CYCLE
 */
std::unordered_map<std::string, int> TEM_REG_LOCKER;

/**
 *  memory lock address, for store commit, load has to wait for another cycle
 */
std::vector<int> JUST_COMMIT_ADDR;

/**
 *  memory lock address map for known address
 */
std::unordered_map<std::string, int> MEMORY_LOCK;

// *
//  *  memory lock address for not known address (REG)
 
// std::unordered_map<std::string, int> MEMORY_LOCK_REG;

/**
 *  used FU of Integer adder
 */
static int INTEGER_FU_USED = 0;

/**
 *  used FU of FP adder
 */
static int FP_ADDER_FU_USED = 0;

/**
 *  used FU of FP multiplier
 */
static int FP_MULT_FU_USED = 0;

/**
 *  used FU of Load/Store unit
 */
static int LS_FU_USED = 0;

/**
 *  row number that the result should be inserted now
 */
static int RESULT_NOW_ROW = 0;

/**
 *  row anchor for each instruction, which is a map
 */
static std::unordered_map<int, std::vector<int> > INSTRS_ANCHOR;

/**
 *  instruction queue
 */
static std::vector<struct instr> INS_QUEUE;

/**
 *  instruction number that should push into queue
 */
static int TO_PUSH_INTO_QUEUE = 1;

/**
 *  branch stall flag
 */
static int branch_stall = FALSE;

/**
 *  stall for integer adder
 */
static int integer_adder_stall = FALSE;


/****************************************************************************************
 *     								              Structure Definition                                *
 ****************************************************************************************/

 /**
  * Reservation Station Structure
  */
 struct RS
 {
 	int BUSY;
 	std::string OP;
  std::string ROB_ENTRY;   /* begin with one, but ROB is an array that begins with zero */
 	float VJ;
 	float VK;
 	std::string QJ;
 	std::string QK;
 	int A;
 };

 /**
  *  Instruction structure
  */
 struct instr
 {
 	std::string _ins;
 	int ex_begin;
 	int mem_begin;
 	int ins_type;
 	int cycle_need;
 	int has_committed;
 	int num;
 	int in_rs;
 	int state;
  int inQ;
 };

 /**
  *  ROB entry structure
  */
 struct rob_entry
 {
 	std::string rob_name;
 	int type;
 	std::string dest;
 	float value;
 	int finish;
 };


/****************************************************************************************
 *     								              Function Definition                                 *
 ****************************************************************************************/

/**
 *  read a file with name, line 3
 */
void
read_file(std::string file_name);

/**
 *  split a string and push elements into pre-constructed vector, line 87
 */
std::vector<std::string>
&split(const std::string &s, char delim, std::vector<std::string> &elems);

/**
 *  split a string and return a vector containing the elements, line 98
 */
std::vector<std::string>
split(const std::string &s, char delim);

/**
 * left trim a string, line 105
 */
static inline std::string
&ltrim(std::string &s);

/**
 * right trim a string, line 112
 */
static inline std::string
&rtrim(std::string &s);

/**
 * trim a string, line 119
 */
static inline std::string
&trim(std::string &s);

/**
 * process an instruction, line 124
 */
void
pro_instr(const std::string &s, std::vector<std::string> &elems);

/**
 *  initial all fields, line 149
 */
void
init_all();

/**
 *  fetching an instruction, line 226
 */
int
fetch_ins(struct instr& INS);

/**
 *  calculate address, line 350
 */
int
cal_addr(std::string operant);

/**
 *  When load and store, whether the address is ready, line 365
 */
int
addr_ready(struct instr& INS);

/**
 *  set operant into RS, 0 for Integer adder, 1 for FP adder, 2 for FP multiplier
 *  num for which entry in RS
 *  operant_num for which operant in the instruction, line 381
 */
void
set_operant(std::string operant, int type, int num, int operant_num);

/**
 *  print RS for debug, line 473
 */
void
print_rs();

/**
 *  print RAT for debug, line 535
 */
void
print_rat();

/**
 *  init all instructions with structure and store in INSTRS array, line 557
 */
void
init_instructions();

/**
 *  check whether rs is available for INSTRS[num]. line 604
 */
int
check_rs_available(struct instr& INS);

/**
 *  issue an instruction, return TURE or FALSE, line 639
 */
int
issue(struct instr& INS);

/**
 *  value available in arf, return TRUE or FALSE, line 676
 */
int
value_available_in_vj_or_qj(struct instr& INS);

/**
 *  value available in RAT and ROB, line 683
 */
int
value_available_in_vk_or_qk(struct instr& INS);

/**
 *  find first operant in instruction, line 696
 */
std::string
first_operant(std::string ins);

/**
 *  execute an instruction, return TRUE or FALSE, line 704
 */
int
execute(struct instr& INS);

/**
 *  move the SHOULD_FETCH instruction into pipeline, line 767
 */
void
move_to_pipeline_do_nothing(std::vector<struct instr>& INS_QUEUE, int num);

/**
 *  do memory part, line 773
 */
int
memory(struct instr& INS);

/**
 *  write back, line 818
 */
int
write_back(struct instr& INS);

/**
 *  do the instruction calculation, line 886
 */
float
do_instr_cal(struct instr& INS);

/**
 *  clear RS entry, line 944
 */
void
clear_rs_entry(int num, int type);

/**
 *  run an instruction in a certain state, line 999
 */
int
run_to_state(struct instr& INS);

/**
 *  commit an instruction, line 1041
 */
int
commit(struct instr& INS);

/**
 *  update RAT and ROB together, line 1060
 */
void
update_rat_rob(std::string operant);

/**
 *  update ROB value in line num, line 934
 */
void
update_rob_value(int num, float res);

/**
 *  reset RAT rename back to zero, line 939
 */
void
reset_rat(std::string operant);

/**
 * print result, line 1169
 */
void
print_result();

/**
 *  print rob, line 1182
 */
void
print_rob();

/**
 *  put all value from ROB to ARF, line 1192
 */
void
refresh_value();

/**
 *  print cons_map, line 1241
 */
void
print_cons_map();

/**
 *  run simulator, line 1447
 */
void
run_simulator();

/**
 *  empty the temporary register locker map, line 1496
 */
void
empty_reg_locker();

/**
 *  release memory locker
 */
void
release_memory_locker();

/**
 *  print memory
 */
void
print_memory();

/**
 *  Store Memory just commit
 */
int
store_memory_just_commit(int address);

/**
 *  print_memory_lock
 */
void
print_memory_lock();

/**
 *  print_just_commit_addr
 */
void
print_just_commit_addr();

/**
 *  lock storing memory address
 */
void 
lock_storing_address(struct instr& INS);

/**
 *  determine whether a branch has been resolved
 */
int 
branch_resolved(int num);

/**
 *  calculate the address to which the branch will jump
 */
int 
cal_branch_addr(struct instr& INS);

/**
 *  print all static value for debugging
 */
void 
print_static_value();

/**
 *  print an instruction for debugging
 */
void
print_instr(struct instr INS);

/**
 *  release after store commit
 */
void
release_after_store_commit(struct instr INS);

/**
 *  print tem_reg_lock
 */
void
print_tem_reg_locker();

// Sd F6, 0(R2)
// Add R1, R1, R2
// Add R2, R2, R2
// Add.d F20, F2, F2













