using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using SoulsFormats;

namespace ParamStructGenerator
{
    public class CParamCodeGen : IParamCodeGen
    {
        public string FileExtension => ".h";
        public bool MultiFile = true;

        public string GenParamCode(PARAM param, string name, bool writeComments)
        {
            StringBuilder sb = new StringBuilder();

            if (writeComments && MultiFile) sb.AppendLine("/* This file was automatically generated from regulation data. */");
            if (MultiFile) sb.AppendLine($@"#ifndef _PARAM_{name}_H
#define _PARAM_{name}_H
#pragma once
#include ""defs/{param.ParamType}.h\""
");
            if (writeComments) sb.AppendLine($@"// Type: {param.ParamType}");
            sb.AppendLine($"typedef {param.ParamType} {name};");
            if (MultiFile) sb.AppendLine("#endif");
            return sb.ToString();
        }

        public string GenParamdefCode(PARAMDEF def, bool writeComments)
        {
            StringBuilder sb = new StringBuilder();

            if (writeComments && MultiFile) sb.AppendLine("/* This file was automatically generated from XML paramdefs. */");
            
            if (MultiFile) sb.AppendLine($@"#ifndef _PARAMDEF_{def.ParamType}_H
#define _PARAMDEF_{def.ParamType}_H
#pragma once
#include <inttypes.h>
");
            if (writeComments)
            {
                sb.AppendLine($@"// Data Version: {def.DataVersion}
// Is Big Endian: {(def.BigEndian ? "True" : "False")}
// Is Unicode: {(def.Unicode ? "True" : "False")}
// Format Version: {def.FormatVersion}");
            }

            sb.AppendLine($"struct _{def.ParamType} {{");

            foreach (PARAMDEF.Field field in def.Fields)
            {
                if (writeComments) sb.AppendLine();

                if (writeComments)
                {
                    if ((field.DisplayName ?? "") != "")
                        sb.AppendLine($"\t// NAME: {field.DisplayName}");
                    if ((field.Description ?? "") != "")
                        sb.AppendLine($"\t// DESC: {field.Description}");
                }

                string fieldName = ParamdefUtils.FieldTypeToStdInt(field.DisplayType);
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

            sb.AppendLine($@"}};
typedef struct _{def.ParamType} {def.ParamType};");
            if (MultiFile) sb.AppendLine("#endif");
            return sb.ToString();
        }

        public string GenCommonHeader(string name, List<string> includeList)
        {
            StringBuilder sb = new StringBuilder();
            sb.AppendLine($@"/* This file was automatically generated. */
#ifndef _COMMON_{name}_H
#define _COMMON_{name}_H
#pragma once
");
            foreach (var header in includeList)
            {
                sb.AppendLine($"#include \"{header}.h\"");
            }

            sb.AppendLine("#endif");
            return sb.ToString();
        }
    }
}
