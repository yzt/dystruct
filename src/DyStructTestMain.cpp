
#include <dystruct/DyStruct.h>
#include <iostream>

using namespace std;

struct Vec3
{
	unsigned long long x, y, z;
};

int main ()
{
	namespace Dy = DyStruct;
	
	Dy::TypeManager tm {};
	
	auto tU64 = tm.createType<Dy::Family::Basic>(Dy::Basic::U64);
	auto tArr50 = tm.createType<Dy::Family::Array>(50U, tU64);
	
	auto tIVec3 = tm.createType<Dy::Family::DyStruct>();
	tIVec3->addField ({tU64, "x"});
	tIVec3->addField ({tU64, "y"});
	tIVec3->addField ({tU64, "z"});

	auto cU64 = tm.compile (tU64);
	auto cArr50 = tm.compile (tArr50);
	auto cIVec3 = tm.compile (tIVec3);

	cout << cArr50->sizeOf() << endl;
	cout << cIVec3->sizeOf() << endl;

	Dy::InstancePtr p = cArr50->createInstance ();
	Dy::InstancePtr q = p;
	Dy::InstancePtr r = cU64->createInstance ();
	Dy::InstancePtr s = cIVec3->createInstance ();
	Dy::InstancePtr t = cIVec3->createInstance ();

	auto x = cIVec3->accessorField<Dy::Basic::U64>("x");
	auto y = cIVec3->accessorField<Dy::Basic::U64>("y");
	auto z = cIVec3->accessorField<Dy::Basic::U64>("z");

	cout << "ivec3 s = (" << x(s) << ", " << y(s) << ", " << z(s) << ")\n";
	cout << "ivec3 t = (" << x(t) << ", " << y(t) << ", " << z(t) << ")\n";

	x(s) = 42;
	y(s) = 69;
	z(s) = 105;
	x(t) = x(s) + y(s) + z(s);
	y(t) = x(s)*x(s) + y(s)*y(s) + z(s)*z(s);
	z(t) = y(t) / 41;

	// This is the equivalent of the above DyStruct code, to compare the generated assembly
	// TL;DR: The generated code for these two blocks are almost the same, except
	//  that for DyStruct field access, the offset needs to be moved into a register,
	//  and the `mov` instruction for reading from/writing to a DyStruct field uses
	//  base and an offset registers, instead of a base register and immediate offset.
	//  i.e. `mov rax, [rdx + rdi]`, instead of `mov rax, [rdx + 16]`
	// {
		auto ss = new Vec3, tt = new Vec3;

		ss->x = x(s);
		ss->y = y(s);
		ss->z = z(s);
		tt->x = ss->x + ss->y + ss->z;
		tt->y = ss->x*ss->x + ss->y*ss->y + ss->z*ss->z;
		tt->z = tt->y / 41;
		cout << tt->x << "," << tt->y << "," << tt->z << "\n";

		cout << "s = (" << x(s) << ", " << y(s) << ", " << z(s) << ")\n";
		cout << "t = (" << x(t) << ", " << y(t) << ", " << z(t) << ")\n";
	// }

	delete tt;
	delete ss;
	t.destroySelf ();
	s.destroySelf ();
	/**q = nullptr;/*/q.nullify ();/**/
	cArr50->destroyInstance (p);
	r.destroySelf ();

	return 0;
};
