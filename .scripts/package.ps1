# Copyright (c) 2023 Sri Lakshmi Kanthan P
#
# This software is released under the MIT License.
# https://opensource.org/licenses/MIT

# Check if current directory has CMakeLists.txt
if (-not (Test-Path ./CMakeLists.txt)) {
  Write-Host "CMakeLists.txt not found in current working directory" -ForegroundColor Red
  exit 1
}

# the First argument is the build type "Release" or "Debug"
$BuildType = $args[0]

# if build type is not "Release" or "Debug" exit
if ($BuildType -ne "Release" -and $BuildType -ne "Debug") {
  Write-Host "Invalid build type $BuildType" -ForegroundColor Red
  exit 1
}

#-------------------------- clipbird package --------------------------#

# Buil the clipbird with release configuration
Write-Host "Building clipbird with Release configuration"
cmake -G "Visual Studio 17 2022" -B ./build && cmake --build ./build --config $BuildType

# clipbird package data directory
$ClipbirDir = "./.installer/packages/clipbird/data"

# clean the package directory items except .gitignore
Write-Host "Cleaning the package directory $ClipbirDir"
Remove-Item -Recurse -Force $ClipbirDir/* -Exclude .gitignore

# Copy the KF5DNSSD.dll to the package
Write-Host "Copying ${env:KDEROOT}/bin/*.dll to $ClipbirDir"
Copy-Item "$env:KDEROOT/bin/*.dll" $ClipbirDir

# Copy All openssl dlls to the package directory
Write-Host "Copying $env:OPENSSL_DIR/bin/*.dll to $ClipbirDir"
Copy-Item "$env:OPENSSL_DIR/bin/*.dll" $ClipbirDir

# Create the package as BuildType (to lower) version
Write-Host "Creating the package as ${BuildType} version"
windeployqt ./build/${BuildType}/clipbird.exe --dir $ClipbirDir --"$(${BuildType}.ToLower())"

# copy the clipbird.exe to the package directory
Write-Host "Copying ./build/${BuildType}/clipbird.exe to $ClipbirDir"
Copy-Item ./build/${BuildType}/clipbird.exe $ClipbirDir

# copy the Logo to the package directory
Write-Host "Copying ./assets/images/logo.png to $ClipbirDir"
Copy-Item ./assets/images/logo.png $ClipbirDir

#-------------------------- bonjour package --------------------------#

# Bonjour Package data directory
$BonjourDir = "./.installer/packages/bonjour/data"

# clean the package directory items except .gitignore
Write-Host "Cleaning the package directory $BonjourDir"
Remove-Item -Recurse -Force $BonjourDir/* -Exclude .gitignore

# bonjour installer as x86 by default
$bonjour_installer = "$env:BONJOUR_DIR/installer/bonjour.msi"

# if system is x64 copy x64 installer as bonjour.msi
if ($env:PROCESSOR_ARCHITECTURE -eq "AMD64") {
  $bonjour_installer = "$env:BONJOUR_DIR/installer/bonjour64.msi"
}

# copy the bonjour installer to the package
Write-Host "Copying $bonjour_installer to $BonjourDir/bonjour.msi"
Copy-Item $bonjour_installer "$BonjourDir/bonjour.msi"

#----------------------- vc_redist package ------------------------#

# vc_redist Package data directory
$VcRedistDir = "./.installer/packages/vcredist/data"

# clean the package directory items except .gitignore
Write-Host "Cleaning the package directory $VcRedistDir"
Remove-Item -Recurse -Force $VcRedistDir/* -Exclude .gitignore

# move the vc_redist installer from $ClipbirDir
Write-Host "Moving $ClipbirDir/vc_redist to $VcRedistDir/vc_redist.exe"
Move-Item "$ClipbirDir/vc_redist*.exe" "$VcRedistDir/vc_redist.exe"

#------------------ qt installer framework -----------------------#

# Config file for qt installer framework
$ConfigFile = "./.installer/configuration/config.xml"

# package directory for qt installer framework
$PackageDir = "./.installer/packages"

# destination directory for qt installer framework
$Destination = "./build/installer/clipbird.exe"

# make sure that the destination directory exists
New-Item -ItemType Directory -Force -Path $Destination

# Clean the destination directory
Write-Host "Cleaning the $Destination file"
Remove-Item -Recurse -Force $Destination

# create binary package with qt installer framework
Write-Host "Creating binary package with qt installer framework"
binarycreator -c "$ConfigFile" -p "$PackageDir" "$Destination"
