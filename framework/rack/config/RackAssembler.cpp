#include "framework/midi/MidiHandler.h"
#include "RackAssembler.h"
using namespace StrangeIO;
using namespace StrangeIO::Config;

RackAssembler::RackAssembler(std::unique_ptr<RackUnitFactory> factory) {
	mUnitFactory = std::move(factory);
}

void RackAssembler::assemble(const RackDesc& desc, Rack& rack) {
	assembleMainlines(desc, rack);
	assembleMidiDevices(desc, rack);
	assembleDaisychains(desc, rack);
}

void RackAssembler::assembleMainlines(const RackDesc& desc, Rack& rack) {
	for(auto mainline : desc.setup.mainlines) {
		rack.addMainline(mainline);
	}
}

void RackAssembler::assembleDaisychains(const RackDesc& desc, Rack& rack) {
	for(auto link : desc.setup.daisychains) {
		checkUnit(desc, rack, link.to);
		rack.connectUnits(link.from, link.plug, link.to, link.jack);
	}
}

const UnitDesc& RackAssembler::unitDescription(const RackDesc& desc, std::string label) {
	for(const auto& unit : desc.setup.units) {
		if(unit.label == label) {
			return unit;
		}
	}

	throw;
}

std::unique_ptr<RackUnit> RackAssembler::assembleUnit(std::string unit, std::string label, std::string target) {
	if(target.empty()) {
		return mUnitFactory->build(unit, label);
	} else {
		return mUnitFactory->load(target, unit, label);
	}
}

void RackAssembler::checkUnit(const RackDesc& desc, Rack& rack, std::string label) {
	auto& ud = unitDescription(desc, label);
	if(!rack.hasUnit(ud.label)) {
		auto u = assembleUnit(ud.unit, ud.label, ud.library);
		
		for(const auto& config : ud.configs) {
			u->setConfig(config.type, config.value);
		}

		if(u->midiControllable()) {
			assembleMidiBindings(desc, rack, *u);
		}
		rack.addUnit(std::move(u));
	}
}

void RackAssembler::assembleMidiBindings(const RackDesc& desc, Rack& rack, RackUnit& unit) {
	auto midi = rack.getMidiHandler();
	auto& bindings = unitDescription(desc, unit.getName()).bindings;
	auto exports = unit.midiExportedMethods();

	for(auto& binding : bindings) {
		auto exp = exports.find(binding.name);
		if(exp != exports.end()) {
			midi.addBinding(binding.module, binding.code, exp->second);
		}
	}
}

void RackAssembler::assembleMidiDevices(const RackDesc& desc, Rack& rack) {
	auto midiHandler = rack.getMidiHandler();
	for(auto device : desc.midi.controllers) {
		midiHandler.addModule(device.port, device.label);
	}
}
