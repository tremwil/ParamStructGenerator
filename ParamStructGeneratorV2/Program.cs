// See https://aka.ms/new-console-template for more information
using ParamStructGenerator;

string paramdefFolder = "C:\\Users\\William\\source\\repos\\Paramdex\\Paramdex\\ER\\Defs";
string regulationPath = "E:\\SteamLibrary\\steamapps\\common\\ELDEN RING\\Game\\regulation.bin";
string outputFolder = ".\\out";

//LibraryGen gen = new LibraryGen() { CodeGen = new CppParamCodeGen() };
CSingleHeaderGen gen = new CSingleHeaderGen() { CEMode = true };
gen.GenerateCode(regulationPath, paramdefFolder, outputFolder);