#include "../GeoGen/GeoGen.hpp"
#include "../GeoGen/compiler/Compiler.hpp"

int main(){
	geogen::Generator g(5);

	geogen::compiler::Compiler compiler;
	geogen::compiler::CompiledScript script = compiler.CompileScript(std::string("yield HeightMap.Empty();"));

	return 0;
}