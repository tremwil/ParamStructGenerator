using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using SoulsFormats;

namespace ParamStructGenerator
{
    public class LibraryGen
    {
        public IParamCodeGen CodeGen { get; set; }

        public void GenerateCode(string regulationPath, string paramdefFolder, string outputFolder)
        {
            var paramdefIncludes = new List<string>();
            string outFile;

            foreach (string file in Directory.GetFiles(paramdefFolder, "*.xml"))
            {
                PARAMDEF def = PARAMDEF.XmlDeserialize(file);
                ParamdefUtils.MakeInternalNamesUnique(def, ParamdefUtils.UniqueNameMethod.Counter);

                paramdefIncludes.Add(def.ParamType);

                Console.Write($"Generating paramdef {def.ParamType}... ");
                WriteAllTextAndCreateDirs(Path.Combine(outputFolder, $"param/defs/{def.ParamType}{CodeGen.FileExtension}"), CodeGen.GenParamdefCode(def, true));
                Console.WriteLine("done");
            }
            WriteAllTextAndCreateDirs(Path.Combine(outputFolder, "param/defs/defs" + CodeGen.FileExtension), CodeGen.GenCommonHeader("defs", paramdefIncludes));

            BND4 archive = SFUtil.DecryptERRegulation(regulationPath);
            var paramIncludes = new List<string>();

            foreach (var file in from f in archive.Files where f.Name.EndsWith(".param") select f)
            {
                string paramName = Path.GetFileNameWithoutExtension(file.Name);

                Console.Write($"Generating param {paramName}... ");
                paramIncludes.Add(paramName);

                PARAM param = PARAM.Read(file.Bytes);
                WriteAllTextAndCreateDirs(Path.Combine(outputFolder, $"param/{paramName}{CodeGen.FileExtension}"), CodeGen.GenParamCode(param, paramName, true));
                Console.WriteLine("done");
            }
            WriteAllTextAndCreateDirs(Path.Combine(outputFolder, "param/params" + CodeGen.FileExtension), CodeGen.GenCommonHeader("params", paramIncludes));
        }

        public void WriteAllTextAndCreateDirs(string path, string text)
        {
            new FileInfo(path).Directory.Create();
            File.WriteAllText(path, text);
        }
    }
}
