
Typical issues during compilation 

The build tools for v141_xp (Platform Toolset = 'v141_xp') cannot be found. To build using the v141_xp build tools, please install v141_xp build tools. Alternatively, you may upgrade to the current Visual Studio tools by selecting the Project menu or right-click the solution, and then selecting "Retarget solution".

Solution:
install the toolset:
Bring up the Visual Studio installer (Tools -> Get Tools and Features)
Select Desktop development with C++
Select Windows XP support for C++ from the Summary section
Click Modify


Error	D8016	'/ZI' and '/Gy-' command-line options are incompatible	tool_text	C:\Users\Ram\Downloads\CLCL-SourceCode\tltxt002_eng\src\tool_text\cl	1	
Solution: Resolved by changing
Project ➔ Properties ➔ C/C++ ➔ General ➔ Debug Information Format ➔ Program Database (/Zi)


Error	RC1015	cannot open include file 'afxres.h'.	tool_text	C:\Users\Ram\Downloads\CLCL-SourceCode\tltxt002_eng\src\tool_text\tool_text.rc	10
Solution: Need to install MFC and ATL for C++  
Tools -> Get Tools and Features... will list what components are installed and allow for adding and removing of components.

	