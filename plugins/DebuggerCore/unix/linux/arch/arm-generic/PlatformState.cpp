/*
Copyright (C) 2017 Ruslan Kabatsayev
                   b7.10110111@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "PlatformState.h"

namespace DebuggerCorePlugin {

const std::array<PlatformState::GPR::RegNameVariants, GPR_COUNT> PlatformState::GPR::GPRegNames = {
	RegNameVariants{"r0" ,"a1" },
	RegNameVariants{"r1" ,"a2" },
	RegNameVariants{"r2" ,"a3" },
	RegNameVariants{"r3" ,"a4" },
	RegNameVariants{"r4" ,"v1" },
	RegNameVariants{"r5" ,"v2" },
	RegNameVariants{"r6" ,"v3" },
	RegNameVariants{"r7" ,"v4" },
	RegNameVariants{"r8" ,"v5" },
	RegNameVariants{"r9" ,"sb", "v6"},
	RegNameVariants{"r10","sl", "v7"},
	RegNameVariants{"r11","fp", "v8"},
	RegNameVariants{"r12","ip", "v6"},
	RegNameVariants{"sp" ,"r13"},
	RegNameVariants{"lr" ,"r14"},
	RegNameVariants{"pc" ,"r15"}
};

PlatformState::PlatformState()
{
	clear();
}

IState *PlatformState::clone() const
{
	auto*const copy=new PlatformState();
	copy->gpr=gpr;
	return copy;
}

QString PlatformState::flags_to_string() const
{
	return "flags string"; // FIXME: stub
}
QString PlatformState::flags_to_string(edb::reg_t flags) const
{
	return "flags string"; // FIXME: stub
}

auto PlatformState::findGPR(QString const& name) const -> decltype(gpr.GPRegNames.begin())
{
	return std::find_if(GPR::GPRegNames.begin(), GPR::GPRegNames.end(),
						[&name](GPR::RegNameVariants const& variants)
						{
							for(const char*const var : variants)
								if(var==name)
									return true;
							return false;
						});
}

Register PlatformState::value(const QString &reg) const
{
	const auto name=reg.toLower();
	if(name=="cpsr")
		return flags_register();
	if(vfp.filled && name=="fpscr")
		return make_Register<32>("fpscr", vfp.fpscr, Register::TYPE_FPU);
	const auto gprFoundIt=findGPR(name);
	if(gprFoundIt!=GPR::GPRegNames.end())
		return gp_register(gprFoundIt-GPR::GPRegNames.begin());
	return Register();
}
Register PlatformState::instruction_pointer_register() const
{
#ifdef EDB_ARM32
	return gp_register(GPR::PC);
#else
	return Register();  // FIXME: stub
#endif
}
Register PlatformState::flags_register() const
{
#ifdef EDB_ARM32
	if(!gpr.filled) return Register();
	return make_Register<32>("cpsr", gpr.cpsr, Register::TYPE_GPR);
#else
	return Register();  // FIXME: stub
#endif
}
edb::address_t PlatformState::frame_pointer() const
{
	return gpr.GPRegs[GPR::FP];
}
edb::address_t PlatformState::instruction_pointer() const
{
	return gpr.GPRegs[GPR::PC];
}
edb::address_t PlatformState::stack_pointer() const
{
	return gpr.GPRegs[GPR::SP];
}
edb::reg_t PlatformState::debug_register(size_t n) const
{
	return 0; // FIXME: stub
}
edb::reg_t PlatformState::flags() const
{
	return gpr.cpsr;
}
void PlatformState::adjust_stack(int bytes)
{
	// FIXME: stub
}
void PlatformState::clear()
{
	gpr.clear();
}
bool PlatformState::empty() const
{
	return gpr.empty();
}
bool PlatformState::GPR::empty() const
{
	return !filled;
}
void PlatformState::GPR::clear()
{
	util::markMemory(this, sizeof(*this));
	filled=false;
}
void PlatformState::set_debug_register(size_t n, edb::reg_t value)
{
	// FIXME: stub
}
void PlatformState::set_flags(edb::reg_t flags)
{
	gpr.cpsr=flags;
}
void PlatformState::set_instruction_pointer(edb::address_t value)
{
	gpr.GPRegs[GPR::PC]=value;
}
void PlatformState::set_register(const Register &reg)
{
	if(!reg) return;
	const auto name=reg.name().toLower();
	if(name=="cpsr")
	{
		set_flags(reg.value<edb::reg_t>());
		return;
	}
	if(name=="fpscr")
	{
		vfp.fpscr=reg.value<decltype(vfp.fpscr)>();
		return;
	}
	const auto gprFoundIt=findGPR(name);
	if(gprFoundIt!=GPR::GPRegNames.end())
	{
		const auto index=gprFoundIt-GPR::GPRegNames.begin();
		assert(index<16);
		gpr.GPRegs[index]=reg.value<edb::reg_t>();
		return;
	}
}
void PlatformState::set_register(const QString &name, edb::reg_t value)
{
#ifdef EDB_ARM32
	const QString regName = name.toLower();
	set_register(make_Register<32>(regName, value, Register::TYPE_GPR));
	// FIXME: this doesn't take into account any 64-bit registers - possibly FPU data?
#endif
}
Register PlatformState::gp_register(size_t n) const
{
#ifdef EDB_ARM32
	assert(n<GPR::GPRegNames.size());
	return make_Register<32>(gpr.GPRegNames[n].front(), gpr.GPRegs[n], Register::TYPE_GPR);
#else
	return Register(); // FIXME: stub
#endif
}

void PlatformState::fillFrom(user_regs const& regs)
{
	for(unsigned i=0;i<gpr.GPRegs.size();++i)
		gpr.GPRegs[i]=regs.uregs[i];
	gpr.cpsr=regs.uregs[16];
	gpr.filled=true;
}

void PlatformState::fillFrom(user_vfp const& regs)
{
	for(unsigned i=0;i<vfp.d.size();++i)
		vfp.d[i]=regs.fpregs[i];
	vfp.fpscr=regs.fpscr;
	vfp.filled=true;
}

void PlatformState::fillStruct(user_regs& regs) const
{
	util::markMemory(&regs, sizeof(regs));
	if(gpr.filled)
	{
		for(unsigned i=0;i<gpr.GPRegs.size();++i)
			regs.uregs[i]=gpr.GPRegs[i];
		regs.uregs[16]=gpr.cpsr;
		// FIXME: uregs[17] is not filled
	}
}

void PlatformState::fillStruct(user_vfp& regs) const
{
	util::markMemory(&regs, sizeof(regs));
	if(vfp.filled)
	{
		for(unsigned i=0;i<vfp.d.size();++i)
			regs.fpregs[i]=vfp.d[i];
		regs.fpscr=vfp.fpscr;
	}
}

}