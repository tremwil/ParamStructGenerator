using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using SoulsFormats;
using System.IO;

namespace ParamStructGenerator
{
    public class ParamStructGenerator
    {
        public static void GenerateFile(string outputFolder, string structName, PARAMDEF param, bool comments = true)
        {
            string filePath = Path.Combine(outputFolder, $"{structName}.h");
            new FileInfo(filePath).Directory.Create();

            using (FileStream fs = File.Open(filePath, FileMode.Create, FileAccess.Write))
            {
                using (StreamWriter sw = new StreamWriter(fs))
                {
                    if (comments) sw.WriteLine("/* NOTE: AUTO-GENERATED CODE. Modify at your own risk. */");
                    sw.WriteLine("#pragma once");
                    sw.WriteLine($"#ifndef _PARAM_{structName}_H");
                    sw.WriteLine($"#define _PARAM_{structName}_H");
                    sw.WriteLine("#include <stdint.h>");
                    sw.WriteLine();

                    StringBuilder sb = new StringBuilder();
                    GenerateStruct(sb, structName, param, comments);
                    sw.WriteLine(sb);
                    sw.WriteLine("#endif");
                }
            }
        }

        public static void GenerateStruct(StringBuilder sb, string structName, PARAMDEF param, bool comments = true)
        {
            if (comments)
            {
                if ((param.ParamType ?? "") != "")
                    sb.AppendLine($"// {param.ParamType}");

                sb.AppendLine($"// Data Version: {param.DataVersion}");
                sb.AppendLine($"// Is Big Endian: {(param.BigEndian ? "True" : "False")}");
                sb.AppendLine($"// Is Unicode: {(param.Unicode ? "True" : "False")}");
                sb.AppendLine($"// Format Version: {param.FormatVersion}");
            }

            sb.AppendLine($"typedef struct _{structName} {{");
            sb.AppendLine();

            int bitOffset = 0;
            for (int i = 0; i < param.Fields.Count; i++)
            {
                PARAMDEF.Field field = param.Fields[i];
                field.InternalName += $"_{bitOffset/8:X3}"; // Necessary as some params have duplicate names

                GenerateField(sb, field, bitOffset, comments);
                sb.AppendLine();

                if (field.BitSize == -1)
                    bitOffset += 8 * (ParamUtil.IsArrayType(field.DisplayType) ? ParamUtil.GetValueSize(field.DisplayType) * field.ArrayLength : ParamUtil.GetValueSize(field.DisplayType));
                else
                    bitOffset += field.BitSize;
            }

            sb.AppendLine($"}} {structName};");
        }

        public static void GenerateField(StringBuilder sb, PARAMDEF.Field field, int bitOffset = -1, bool comments = true)
        {
            if (comments)
            {
                if ((field.DisplayName ?? "") != "")
                    sb.AppendLine($"\t// NAME: {field.DisplayName}");
                if ((field.Description ?? "") != "")
                    sb.AppendLine($"\t// DESC: {field.Description}");
            }

            string fieldName = FieldTypeToStdInt(field.DisplayType);
            bool isZeroSize = false;

            StringBuilder fieldBuilder = new StringBuilder();
            fieldBuilder.Append($"\t{fieldName} {field.InternalName}");

            if (ParamUtil.IsBitType(field.DisplayType) && field.BitSize > 0)
                fieldBuilder.Append($": {field.BitSize}");
            else if (field.BitSize != -1)
                isZeroSize = true;
            else if (ParamUtil.IsArrayType(field.DisplayType) && field.ArrayLength > 0)
                fieldBuilder.Append($"[{field.ArrayLength}]");
            else if (field.ArrayLength <= 0)
                isZeroSize = true;

            // Comment out the field if it has zero size
            if (isZeroSize) sb.Append("\t// ");
            sb.AppendLine($"{fieldBuilder};");
        }

        public static string FieldTypeToStdInt(PARAMDEF.DefType defType)
        {
            switch (defType)
            {
                case PARAMDEF.DefType.u8:
                case PARAMDEF.DefType.dummy8: return "uint8_t";
                case PARAMDEF.DefType.s8: return "int8_t";
                case PARAMDEF.DefType.u16: return "uint16_t";
                case PARAMDEF.DefType.s16: return "int16_t";
                case PARAMDEF.DefType.u32: return "uint32_t";
                case PARAMDEF.DefType.s32: return "int32_t";
                case PARAMDEF.DefType.f32: return "float";

                case PARAMDEF.DefType.fixstr: return "char";
                case PARAMDEF.DefType.fixstrW: return "wchar_t";
            }
            return "unknown_type";
        }
    }
}
