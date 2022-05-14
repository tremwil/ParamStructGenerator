using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using SoulsFormats;
using System.Runtime.InteropServices;

namespace ParamStructGenerator
{
    [StructLayout(LayoutKind.Sequential)]
    public struct FieldBitmask
    {
        public UInt32 offset;
        public UInt32 uid; // 0 = terminator
        public UInt64 bitmask;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct FieldMetadataEntry
    {
        public UInt32 nameOffset; // 0 = terminator
        public UInt32 fieldBitmaskOffset;
    }

    public class CSingleHeaderGen
    {
        public bool CEMode { get; set; } = false;
        public IParamCodeGen CodeGen = new CParamCodeGen() { MultiFile = false };

        public void GenerateCode(string regulationPath, string paramdefFolder, string outputFolder)
        {
            StringBuilder header = new StringBuilder();
            header.AppendLine(@"/* This file was automatically generated from paramdef XMLs and game regulation data.*/
#ifndef CPARAMS_H
#pragma once
#define CPARAMS_H
");
            if (CEMode) header.AppendLine(@"typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;
typedef short wchar_t;
");
            List<PARAMDEF> defs = new List<PARAMDEF>();

            foreach (string file in Directory.GetFiles(paramdefFolder, "*.xml"))
            {
                PARAMDEF def = PARAMDEF.XmlDeserialize(file);
                ParamdefUtils.MakeInternalNamesUnique(def, ParamdefUtils.UniqueNameMethod.Counter);
                defs.Add(def);
            }

            foreach (PARAMDEF def in defs)
            {
                Console.Write($"Generating paramdef {def.ParamType}... ");
                header.AppendLine(CodeGen.GenParamdefCode(def, false));
                Console.WriteLine("done");
            }

            Console.Write("Generating field metadata...");
            File.WriteAllBytes(Path.Combine(outputFolder, "param_fields.bin"), GenMetadata(defs));
            Console.WriteLine("done");

            BND4 archive = SFUtil.DecryptERRegulation(regulationPath);

            foreach (var file in from f in archive.Files where f.Name.EndsWith(".param") select f)
            {
                string paramName = Path.GetFileNameWithoutExtension(file.Name);

                Console.Write($"Generating param {paramName}... ");

                PARAM param = PARAM.Read(file.Bytes);
                header.Append(CodeGen.GenParamCode(param, paramName, false));
                Console.WriteLine("done");
            }
            header.AppendLine("#endif");
            header.AppendLine(File.ReadAllText("CEParamUtilsLib\\CParamUtils.h"));
            WriteAllTextAndCreateDirs(Path.Combine(outputFolder, "params.h"), header.ToString());
        }

        public void WriteAllTextAndCreateDirs(string path, string text)
        {
            new FileInfo(path).Directory.Create();
            File.WriteAllText(path, text);
        }

        public static byte[] GenMetadata(List<PARAMDEF> defs)
        {
            // First pass: Generates the array of field bitmasks, and compute offsets relative to start of field bitmask array

            int fieldBitmaskRelOffset = 0;

            List<FieldMetadataEntry> entries = new List<FieldMetadataEntry>();
            List<string> entryNames = new List<string>();
            List<FieldBitmask> bitmasks = new List<FieldBitmask>();

            foreach (PARAMDEF def in defs)
            {
                int fieldBitOffset = 0;
                uint numAddedField = 0;

                foreach (PARAMDEF.Field field in def.Fields)
                {
                    int fieldBitSize = (field.BitSize == -1) ? 8 * (ParamUtil.IsArrayType(field.DisplayType) ? ParamUtil.GetValueSize(field.DisplayType) * field.ArrayLength : ParamUtil.GetValueSize(field.DisplayType)) : field.BitSize;

                    if (fieldBitSize > 1 && field.DisplayType != PARAMDEF.DefType.dummy8)
                    {
                        numAddedField++;
                        while (fieldBitSize > 0)
                        {
                            int offset = fieldBitOffset / 64;
                            int maskBegin = fieldBitOffset % 64;
                            int maskSize = Math.Min(64 - (fieldBitOffset % 64), fieldBitSize);
                            ulong mask = unchecked((1ul << maskSize) - 1ul) << maskBegin;
                            bitmasks.Add(new FieldBitmask() { offset = (uint)offset * 8, bitmask = mask, uid = numAddedField });

                            fieldBitSize -= maskSize;
                            fieldBitOffset += maskSize;
                        }
                    }
                    else fieldBitOffset += fieldBitSize;
                }
                // Only add this metadata for defs that need it
                if (numAddedField != 0)
                {
                    // terminator bitmask 
                    bitmasks.Add(new FieldBitmask() { offset = 0, bitmask = 0, uid = 0 });

                    entries.Add(new FieldMetadataEntry() { fieldBitmaskOffset = (uint)fieldBitmaskRelOffset, nameOffset = 0 });
                    entryNames.Add(def.ParamType);

                    fieldBitmaskRelOffset += (int)(numAddedField + 1) * Marshal.SizeOf<FieldBitmask>();
                }
            }
            // terminator entry
            entries.Add(new FieldMetadataEntry() { fieldBitmaskOffset = 0, nameOffset = 0 });


            // Second pass: Compute total required memory, and serialize to bytes

            int fieldArrayOffset = entries.Count() * Marshal.SizeOf<FieldMetadataEntry>();
            int nameOffset = fieldArrayOffset + bitmasks.Count() * Marshal.SizeOf<FieldBitmask>();
            int totalDataSize = nameOffset + entryNames.Sum(x => x.Length + 1);

            byte[] buff = new byte[totalDataSize];
            using (MemoryStream m = new MemoryStream(buff, true))
            {
                using (BinaryWriter bw = new BinaryWriter(m))
                {
                    int currNameOffset = nameOffset;
                    for (int i = 0; i < entries.Count; i++)
                    {
                        FieldMetadataEntry entry = entries[i];
                        if (i != entries.Count - 1)
                        {
                            entry.fieldBitmaskOffset += (uint)fieldArrayOffset;
                            entry.nameOffset += (uint)currNameOffset;
                            currNameOffset += entryNames[i].Length + 1;
                        }
                        bw.Write(StructToBytes(entry));
                    }
                    foreach (FieldBitmask fbm in bitmasks)
                        bw.Write(StructToBytes(fbm));

                    foreach (string name in entryNames)
                    {
                        bw.Write(Encoding.UTF8.GetBytes(name));
                        bw.Write((byte)0);
                    }
                }
            }
            return buff;
        }

        private static byte[] StructToBytes<T>(T pod) where T : struct
        {
            int size = Marshal.SizeOf<T>();
            byte[] buff = new byte[size];

            IntPtr mem = Marshal.AllocHGlobal(size);
            Marshal.StructureToPtr(pod, mem, true);
            Marshal.Copy(mem, buff, 0, size);
            Marshal.FreeHGlobal(mem);
            return buff;
        }
    }
}
