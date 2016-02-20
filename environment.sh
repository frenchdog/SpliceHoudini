echo "Setting up SpliceHoudini environment:"

if [ -z ${QT_DIR+x} ]
then
echo "Please set QT_DIR path"
else

SPLICEHOUDINI_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
export SPLICEHOUDINI_DIR
echo "  Set SPLICEHOUDINI_DIR=\"$SPLICEHOUDINI_DIR\""
 
export FABRIC_EXTS_PATH=$FABRIC_EXTS_PATH:$SPLICEHOUDINI_DIR/Exts
echo "  Add SpliceHoudini Exts. FABRIC_EXTS_PATH=\"$FABRIC_EXTS_PATH\""

export FABRIC_DFG_PATH=$FABRIC_DFG_PATH:$SPLICEHOUDINI_DIR/DFG_Presets
echo "  Add SpliceHoudini DFG Presets. FABRIC_DFG_PATH=\"$FABRIC_DFG_PATH\""

unamestr=`uname -s`
if [[ "$unamestr" == 'Linux' ]]; then
	export LD_LIBRARY_PATH=$FABRIC_DIR/lib
	echo "  Set LD_LIBRARY_PATH=\"$LD_LIBRARY_PATH\""
elif [[ "$unamestr" == 'Darwin' ]]; then
	export DYLD_LIBRARY_PATH=$FABRIC_DIR/lib
	echo "  Set DYLD_LIBRARY_PATH=\"$DYLD_LIBRARY_PATH\""
fi

fi

echo "Done!"
