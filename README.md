# ParamStructGenerator
Automatically generate C/C++ structs from XML paramdefs and a regulation file. Super untested, the only thing I made sure of is that they compiled with msvc. Right now it will only work for ER as the regulation parsing was hardcoded, but it should be easy to modify it for other games.

### [Download the headers (ER paramdefs only)](https://github.com/tremwil/ParamStructGenerator/blob/master/pre_generated_headers.zip?raw=true)

# Planned features (mabye)
- Turn this into a proper command-line tool
- Generate TBF/name enums, and use TBF enum types in paramdefs whenever possible
- Turn this into a proper library that can be used to generate code for multiple target languages (probably won't happen unless I need it)

# Credits
- [SoulsFormats](https://github.com/JKAnderson/SoulsFormats) by [JKAnderson](https://github.com/JKAnderson), ([vawser](https://github.com/vawser)'s build for Yapped, modified to expose `ParamUtil`)
- [Nordgaren](https://github.com/Nordgaren) for English-Japanese paramdefs (from his amazing Elden Ring trainer you can find [here](https://github.com/Nordgaren/Elden-Ring-Debug-Tool))
- [Paramdex](https://github.com/soulsmods/Paramdex) for original Japanese paramdefs. 
