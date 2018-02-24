#include <iostream>
#include <algorithm>
#include <algorithm>
#include <vector>
#include <boost/asio.hpp>

using namespace std;

int main()
{
	vector<int> t{1,2,3};
	cout<<*find(t.begin(),t.end(),3)<<endl;
	
	std::cout<<"hello"<<endl;
	return 0;
}