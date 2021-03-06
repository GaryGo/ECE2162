#include <iostream>
#include <vector>

using namespace std;

struct T
{
	string a;
	int val;
};


int main()
{
	vector<struct T> map;
	for (int i = 0; i < 2; i++)
	{
		if (i == 0)
		{
			struct T tmp;
			tmp.a = "abgc";
			tmp.val = 1;
			map.push_back(tmp);
		}
		else
		{
			struct T tmp;
			tmp.a = "defg";
			tmp.val = 2;
			map.push_back(tmp);
		}
	}
	for (auto it = map.begin(); it != map.end(); ++it)
	{
		cout << it->a << "	" << it->val << endl;
	}
	
	return 0;
}

// av acd  efd efdsa dfef efd se ef 

// a v  4	 1 		 1
// b c  3	 4       1
// c d  2   15      1
// d e  5   1   5   1

// rob entries = 64
// R1=12, R2=32, F20=3.0, R3=12, R4=32
// Mem[4]=3.0, Mem[8]=2.0, Mem[12]=1.0, Mem[24]=6.0, Mem[28]=5.0, Mem[32]=4.0

// Ld F2, 0(R1)
// Mult.d F4, F2, F20
// Ld F6, 0(R2)
// Add.d F6, F4, F6
// Sd F6, 0(R2)
// Addi R3, R3, 5
// Addi R4, R4, -5
// Bne R3, R4, -8
// Add.d F20, F2, F2



// Addi R3, R3, 12
// Mult.d R6, R5, R1
// Beq R3, R6, -3


// Addi R3, R3, 10
// Addi R4, R4, -10
// Beq R3, R4, -3