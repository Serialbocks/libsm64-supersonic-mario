import os
from os.path import exists
from shutil import copyfile

def main():
    outputBasePath = 'dist/'
    dllPath = outputBasePath + 'sm64.dll'
    exportsPath = outputBasePath + 'sm64.exports'
    defPath = outputBasePath + 'sm64.def'
    libPath = outputBasePath + 'sm64.lib'

    vsToolsBasePaths = [
        'C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\\bin\\',
        'C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Tools\\MSVC\\14.29.30133\\bin\\Hostx64\\x64\\'
    ]

    vsToolsBasePath = ""
    for basePath in vsToolsBasePaths:
        if os.path.isdir(basePath):
            vsToolsBasePath = basePath
            break

    if vsToolsBasePath == "":
        print('ERROR: VS tools path not found. Exiting.')
        return

    dumpbinPath = '\"' + vsToolsBasePath + 'dumpbin\"'
    libCommandPath = '\"' + vsToolsBasePath + 'lib.exe\"'

    bakkesmodLibDir = 'C:\\Users\\John\\AppData\\Roaming\\bakkesmod\\bakkesmod\libs\\'
    bakkesmodDllPath = bakkesmodLibDir + 'sm64.dll'
    bakkesmodLibPath = bakkesmodLibDir + 'sm64.lib'

    if not exists(dllPath):
        print('{} not found'.format(dllPath))
        return
    
    print('generating exports file...')
    os.system('{} /EXPORTS {} > {}'.format(dumpbinPath, dllPath, exportsPath))

    print('generating def file...')
    exportsFile = open(exportsPath, 'r')
    exportsLines = exportsFile.readlines()
    exportsFile.close()
    i = 0
    while i < 14:
        exportsLines.pop(0)
        i = i + 1
    
    numFunctions = int(exportsLines[0].strip().split(' ')[0])
    i = 0
    while i < 4:
        exportsLines.pop(0)
        i = i + 1

    defFile = open(defPath, 'w')
    defFile.write('EXPORTS')
    i = 0
    while i <= numFunctions:
        split = exportsLines[i].strip().split(' ')
        if(len(split) > 5):
            funcName = split[6].strip()
            defFile.write(funcName)
            if i != numFunctions:
                defFile.write('\n')
        else:
            defFile.write('\n')
        i = i + 1
    defFile.close()

    print('generating exp and lib files...')
    os.system('{} /def:{} /machine:x64 /out:{}'.format(libCommandPath, defPath, libPath))

    print('copying dll and lib to bakkesmod...')
    copyfile(dllPath, bakkesmodDllPath)
    copyfile(libPath, bakkesmodLibPath)

main()