#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/Triple.h"
#include "llvm/DebugInfo/DIContext.h"
#include "llvm/DebugInfo/DWARF/DWARFContext.h"
#include "llvm/DebugInfo/DWARF/DWARFFormValue.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Object/RelocVisitor.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Dwarf.h"
#include <algorithm>
#include <cstring>
#include <list>
#include <string>
#include <system_error>
#include <iostream>
#include "Variable.hpp"
#include "CommonBlock.hpp"
#include "Subprogram.hpp"

using namespace llvm;
using namespace object;

static cl::list<std::string>
InputFilenames(cl::Positional, cl::desc("<input object files>"),
               cl::ZeroOrMore);

static int ReturnValue = EXIT_SUCCESS;

static bool error(StringRef Filename, std::error_code EC) {
  if (!EC)
    return false;
  errs() << Filename << ": " << EC.message() << "\n";
  ReturnValue = EXIT_FAILURE;
  return true;
}

static void handleSubprogram(const DWARFDebugInfoEntryMinimal *die, DWARFCompileUnit *cu)
{
    auto debug_info_data = cu->getDebugInfoExtractor();
    auto offset = die->getOffset();

    const char *subName = die->getName(cu, llvm::DINameKind::ShortName);

    // ignore main
    if (!strcmp(subName, "main")) return;
    
    Subprogram::Handle r(Subprogram::extract(die, cu));
    
    std::cout << "subroutine " << subName << " @ " << die->getOffset() << std::endl;
    auto same = cu->getDIEForOffset(offset);
    assert(same == die);
    
    auto child = die->getFirstChild();
    while (child && !child->isNULL()) {
        offset = child->getOffset();
        assert(debug_info_data.isValidOffset(offset));
        auto abbrCode = debug_info_data.getULEB128(&offset);
        assert(abbrCode);
        if (child->getAbbreviationDeclarationPtr()) {
            auto tag = child->getTag();
            if (tag == dwarf::DW_TAG_common_block) {
                CommonBlock::extractAndAdd(child, cu);
            }
            else if (tag == dwarf::DW_TAG_formal_parameter) {
                Variable::Handle h = Variable::extract(Variable::PARAMETER, child, cu);
                outs() << "    " << *h;
                //handleParameter(child, cu);
            }
        }
        child = child->getSibling();
    }
}


/**
 * Traverse the graph looking for common blocks and subprograms.
 * Assume the starting node is an object file with a single compile unit.
 * Immediate children of the compile uniit will be subprograms.
 * Immediate children of the subprograms will be the common blocks.

 http://llvm.org/doxygen/classllvm_1_1DWARFDebugInfoEntryMinimal.html
 http://www.dwarfstd.org/doc/DWARF4.pdf
 */
static void extractObject(ObjectFile &obj)
{
    std::unique_ptr<DWARFContextInMemory> DICtx(new DWARFContextInMemory(obj));
    
    auto cuit = DICtx->compile_units();
    for (auto &cu : cuit) {
        
        // calling this with false reads in the entire DIE list for this cu so we can walk through it.
        auto cudie = cu->getUnitDIE(false);
        
        // ensure compilation unit is fortran
        DWARFFormValue form;
        cudie->getAttributeValue(cu.get(), dwarf::DW_AT_language, form);
        auto lang = form.getAsUnsignedConstant().getValueOr(-1);
        if (!(lang == dwarf::DW_LANG_Fortran77 ||
              lang == dwarf::DW_LANG_Fortran90 ||
              lang == dwarf::DW_LANG_Fortran95)) {
            outs() << cudie->getName(cu.get(), DINameKind::ShortName) << " is not FORTRAN 77,90, or 95.  Skipping\n";
            continue;
        }
        
        // look for children of the compile unit that are subprograms
        // immediate children of the subprogram include parameters, common blocks, and local variables
        auto die = cudie->getFirstChild();
        while (die && !die->isNULL()) {
            if (die->isSubprogramDIE()) {
                Subprogram::Handle sub = Subprogram::extract(die, cu.get());
                handleSubprogram(die, cu.get());
                outs() << sub->cDeclaration();
            }
            die = die->getSibling();
        }
    }
}

int main(int argc, char **argv) {
    // Print a stack trace if we signal out.
    sys::PrintStackTraceOnErrorSignal();
    PrettyStackTraceProgram X(argc, argv);
    llvm_shutdown_obj Y;  // Call llvm_shutdown() on exit.
    
    cl::ParseCommandLineOptions(argc, argv, "f2h\n");
    
    // Defaults to a.out if no filenames specified.
    if (InputFilenames.size() == 0) {
        errs() << "no input files specified" << '\n';
        return EXIT_FAILURE;
    }
    
    for (StringRef filename : InputFilenames) {
        ErrorOr<std::unique_ptr<MemoryBuffer>> BuffOrErr = MemoryBuffer::getFileOrSTDIN(filename);
        if (error(filename, BuffOrErr.getError())) {
            errs() << "failed to open " << filename << '\n';
            continue;
        }
        std::unique_ptr<MemoryBuffer> Buff = std::move(BuffOrErr.get());
        
        ErrorOr<std::unique_ptr<ObjectFile>> ObjOrErr =
        ObjectFile::createObjectFile(Buff->getMemBufferRef());
        if (error(filename, ObjOrErr.getError())) {
            errs() << "failed to create object file " << filename << '\n';
            continue;
        }
        ObjectFile &Obj = *ObjOrErr.get();
        outs() << "Extracting " << filename << "\n=========================\n";
        extractObject(Obj);
    }
    return ReturnValue;
}
