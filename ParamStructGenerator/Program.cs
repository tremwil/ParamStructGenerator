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
            string sourceFolder = diagFolder.FileName;

            diagFolder.Title = "Select output directory";
            if (diagFolder.ShowDialog() != CommonFileDialogResult.Ok) { return; }
            string outputFolder = diagFolder.FileName;

            using (FileStream fs = File.Open(Path.Combine(outputFolder, "paramdefs.h"), FileMode.Create, FileAccess.Write))
            {
                using (StreamWriter sw = new StreamWriter(fs))
                {
                    sw.Write(@"/* NOTE: AUTO-GENERATED CODE. Modify at your own risk. */
#pragma once
#ifndef _PARAMDEFS_H
#define _PARAMDEFS_H

");

                    foreach (string file in Directory.GetFiles(sourceFolder, "*.xml"))
                    {
                        string structName = Path.GetFileNameWithoutExtension(file);
                        Console.Write($"Generating {structName}.h...");

                        PARAMDEF def = PARAMDEF.XmlDeserialize(file);
                        ParamStructGenerator.GenerateFile(outputFolder, structName, def, true);

                        sw.WriteLine($"#include \"{structName}.h\"");
                        Console.WriteLine("done");
                    }

                    sw.WriteLine();
                    sw.WriteLine("#endif");
                }
            }
        }
    }
}
