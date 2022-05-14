using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using SoulsFormats;
using System.IO;
using Microsoft.WindowsAPICodePack.Dialogs;
using System.Xml;

namespace ParamStructGenerator
{
    internal class Program
    {
        [STAThread]
        static void Main(string[] args)
        {
            var diagFolder = new CommonOpenFileDialog()
            {
                Title = "Select paramdefs directory",
                IsFolderPicker = true,
                InitialDirectory = Environment.CurrentDirectory
            };
            if (diagFolder.ShowDialog() != CommonFileDialogResult.Ok) { return; }
            string paramdefFolder = diagFolder.FileName;

            diagFolder.IsFolderPicker = false;
            diagFolder.Title = "Select game regulation file";
            if (diagFolder.ShowDialog() != CommonFileDialogResult.Ok) { return; }
            string regulationPath = diagFolder.FileName;

            diagFolder.IsFolderPicker = true;
            diagFolder.Title = "Select output directory";
            if (diagFolder.ShowDialog() != CommonFileDialogResult.Ok) { return; }
            string outputFolder = diagFolder.FileName;

            //LibraryGen gen = new LibraryGen() { CodeGen = new CppParamCodeGen() };
            CSingleHeaderGen gen = new CSingleHeaderGen() { CEMode = true };
            gen.GenerateCode(regulationPath, paramdefFolder, outputFolder);
        }
    }
}
