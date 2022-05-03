using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using SoulsFormats;

namespace ParamStructGenerator
{
    public class CppParamCodeGen : IParamCodeGen
    {
        public string FileExtension => ".h";

        public string GenParamCode(PARAM param, string name, bool writeComments)
        {
            StringBuilder sb = new StringBuilder();

            sb.AppendLine($@"/* This file was automatically generated from regulation data. */
#ifndef _PARAM_{name}_H
#define _PARAM_{name}_H
#pragma once
#include ""defs/{param.ParamType}.h""
");
            if (writeComments) sb.AppendLine($@"// Type: {param.ParamType}");
            sb.AppendLine($@"struct {name} : {param.ParamType} {{
    static constexpr const char* param_type = ""{param.ParamType}"";
    static constexpr const char* param_name = ""{name}"";
}};
");
            if (param.DetectedSize != -1)
                sb.AppendLine($"static_assert(sizeof({name}) == {param.DetectedSize}, \"{name} paramdef size does not match detected size\");");

            sb.AppendLine("#endif");
            return sb.ToString();
        }

        public string GenParamdefCode(PARAMDEF def, bool writeComments)
        {
            StringBuilder sb = new StringBuilder();

            sb.AppendLine($@"/* This file was automatically generated from XML paramdefs. */
#ifndef _PARAMDEF_{def.ParamType}_H
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

            sb.AppendLine($"struct {def.ParamType} {{");

            foreach (PARAMDEF.Field field in def.Fields)
            {
                sb.AppendLine();

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

            sb.AppendLine(@"};

#endif");
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
