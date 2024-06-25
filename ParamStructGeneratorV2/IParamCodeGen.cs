using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using SoulsFormats;

namespace ParamStructGenerator
{
    public interface IParamCodeGen
    {
        string FileExtension { get; }

        string GenParamCode(PARAM param, string name, bool writeComments);

        string GenParamdefCode(PARAMDEF def, bool writeComments);

        string GenCommonHeader(string name, List<string> includeList);
    }
}
