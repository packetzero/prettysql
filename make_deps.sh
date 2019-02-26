TOP=`dirname $0`
mkdir -p ${TOP}/deps
cd ${TOP}/deps
git clone https://github.com/packetzero/dyno.git
git clone https://github.com/packetzero/simplesql.git
BREW=`which brew`
if [ "$BREW" == "" ] ; then
  echo "new homebrew 'brew' command available, will not add rapidjson and anltr4-cpp-runtime"
else
  ${BREW} install rapidjson antlr4-cpp-runtime
fi
