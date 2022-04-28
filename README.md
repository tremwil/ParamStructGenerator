# ParamStructGenerator
Automatically generate C structs from XML paramdefs. Super untested, the only thing I made sure of is that they compiled with msvc. 

# Download the headers
### [Japanese names/descriptions](https://github.com/tremwil/ParamStructGenerator/blob/master/paramdefs_jp.zip?raw=true)
### [English-Japanese names/descriptions (auto-translated)](https://github.com/tremwil/ParamStructGenerator/blob/master/paramdefs_en.zip?raw=true)

# Planned features (mabye)
- Generate TBF/name enums, and use TBF enum types in paramdefs whenever possible
- Turn this into a proper library that can be used to generate code for multiple target languages (probably won't happen unless I need it)

# Credits
- [SoulsFormats](https://github.com/JKAnderson/SoulsFormats) by [JKAnderson](https://github.com/JKAnderson) (modified to expose `ParamUtil`)
- [Nordgaren](https://github.com/Nordgaren) for English-Japanese paramdefs (from his amazing Elden Ring trainer you can find [here](https://github.com/Nordgaren/Elden-Ring-Debug-Tool))
- [Paramdex](https://github.com/soulsmods/Paramdex) for original Japanese paramdefs. 
