#include <iomanip>

#include "RendererDebugger.hpp"
#include "../GeoGen/utils/StringUtils.hpp"
#include "ConsoleUtils.hpp"
#include "renderer_commands/HelpRendererCommand.hpp"
#include "renderer_commands/QuitRendererCommand.hpp"
#include "renderer_commands/RenderingSequenceRendererCommand.hpp"
#include "renderer_commands/RunRendererCommand.hpp"
#include "renderer_commands/StepRendererCommand.hpp"
#include "renderer_commands/StopRendererCommand.hpp"

using namespace geogen;
using namespace compiler;
using namespace runtime;
using namespace console;
using namespace renderer;
using namespace utils;
using namespace std;
using namespace renderer_commands;
using namespace instructions;

RendererDebugger::RendererDebugger(geogen::IStream& in, geogen::OStream& out, renderer::RenderingSequence const& sequence)
: renderer(sequence), in(in), out(out)
{
	this->commandTable.AddCommand(new HelpRendererCommand());
	this->commandTable.AddCommand(new QuitRendererCommand());
	this->commandTable.AddCommand(new RenderingSequenceRendererCommand());
	this->commandTable.AddCommand(new RunRendererCommand());
	this->commandTable.AddCommand(new StepRendererCommand());
	this->commandTable.AddCommand(new StopRendererCommand());

}

void RendererDebugger::Run()
{
	out << std::endl << GG_STR("Starting renderer debugger. ") << std::endl;

	this->GetRenderer()->CalculateRenderingBounds();

	out << std::endl << GG_STR("Calculated rendering bounds. ") << std::endl << std::endl;

	renderer::RenderingStep const* currentStep = this->renderer.GetNextRenderingStep();
	out << std::endl << GG_STR("Next step: ") << (currentStep->ToString()) << GG_STR(" on line ") << currentStep->GetLocation().GetLine() << GG_STR(", column ") << currentStep->GetLocation().GetColumn() << GG_STR(". ") << std::endl << std::endl;

	String input = "";
	while (renderer.GetStatus() == RENDERER_STATUS_READY)
	{
		out << GG_STR("RENDERER>> ");

		getline<Char>(in, input);

		size_t separatorPosition = input.find(" ");
		string commandCue = input.substr(0, separatorPosition);
		string args = "";
		if (separatorPosition != String::npos)
		{
			args = input.substr(separatorPosition + 1);
		}

		Command const* command = this->commandTable.GetCommand(commandCue);
		if (command == NULL)
		{
			out << GG_STR("Unknown command. Use \"h\" to print list of available commands.") << endl << endl;
		}
		else
		{
			dynamic_cast<RendererCommand const*>(command)->Run(this, args);
		}

		if (this->aborted)
		{
			out << GG_STR("Renderer debugger aborted.") << endl << endl;
			return;
		}
	}

	out << GG_STR("Renderer debugger finished.") << endl << endl;
}