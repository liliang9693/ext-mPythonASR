
//% color="#4E9C9C" iconWidth=50 iconHeight=40
namespace mPythonASR {

    //% block="ASR Record for [TIME] seconds and identify once" blockType="command"
    //% TIME.shadow="range" TIME.params.min=1  TIME.params.max=4 TIME.defl=2
    export function ASRStart(parameter: any, block: any) {
        let time = parameter.TIME.code;
        Generator.addInclude('include_MPython_ASR', '#include <MPython_ASR.h>');
        Generator.addObject(`audio`, `MPython_ASR`, `mpythonAsr;`);
        Generator.addObject(`str_ASR_result`, `String`, `str_mpythonAsr_result;`);
        Generator.addCode(`str_mpythonAsr_result=mpythonAsr.getAsrResult(${time});`);
    }

   //% block="ASR get Result" blockType="reporter"
   export function ASRReadResult(parameter: any, block: any) {
    Generator.addInclude('include_MPython_ASR', '#include <MPython_ASR.h>');
    Generator.addObject(`audio`, `MPython_ASR`, `mpythonAsr;`);
    Generator.addObject(`str_ASR_result`, `String`, `str_mpythonAsr_result;`);
    Generator.addCode([`str_mpythonAsr_result`, Generator.ORDER_UNARY_POSTFIX]);

    }

   //% block="ASR [STR] in results?" blockType="boolean"
   //% STR.shadow="string" STR.defl="hello"
   export function ASRCompare(parameter: any, block: any) {
    let str = parameter.STR.code;
    Generator.addInclude('include_MPython_ASR', '#include <MPython_ASR.h>');
    Generator.addObject(`audio`, `MPython_ASR`, `mpythonAsr;`);
    Generator.addObject(`str_ASR_result`, `String`, `str_mpythonAsr_result;`);
    Generator.addCode([`(String(str_mpythonAsr_result).indexOf(String(${str})) != -1)`, Generator.ORDER_UNARY_POSTFIX]);

    }


}
