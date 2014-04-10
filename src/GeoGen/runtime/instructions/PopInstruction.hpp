#pragma once

#include "Instruction.hpp"

namespace geogen 
{
	namespace runtime
	{
		namespace instructions
		{
			class PopInstruction : public Instruction
			{
			private:
			public:				
				PopInstruction(CodeLocation location) : Instruction(location)
				{
				}

				virtual void Serialize(std::iostream& stream) const { stream << "Pop" << std::endl; }

				virtual std::string GetInstructionName() const { return "Pop"; };

				virtual InstructionStepResult Step(VirtualMachine* vm) const;
			};
		}
	}
}