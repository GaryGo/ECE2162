#include <iostream>

using namespace std;

int main()
{
	string str("-3");
	cout << atoi(str.c_str()) << endl;
	return 0;
}

// Ld F2, 0(R1)
// Mult.d F4, F2, F20
// Ld F6, 0(R2)
// Add.d F6, F4, F6
// Sd F6, 0(R2)
// Addi R1, R1, 20
// Add R2, R2, R2
// Add.d F20, F2, F2