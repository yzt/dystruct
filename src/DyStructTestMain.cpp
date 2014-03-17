
#include <dystruct/DyStruct.h>
#include <iostream>

using namespace std;

int main ()
{
	namespace Dy = DyStruct;
	
	Dy::TypeManager tm {};
	
	auto tU64 = tm.createType<Dy::Family::Basic>(Dy::Basic::U64);
	auto tArr50 = tm.createType<Dy::Family::Array>(50U, tU64);

	auto cU64 = tm.compile (tU64);
	auto cArr50 = tm.compile (tArr50);

	cout << cArr50->sizeOf() << endl;

	Dy::InstancePtr p = cArr50->createInstance ();
	Dy::InstancePtr q = p;
	Dy::InstancePtr r = cU64->createInstance ();

	/**q = nullptr;/*/q.nullify ();/**/
	cArr50->destroyInstance (p);
	r.destroySelf ();

	return 0;
};
