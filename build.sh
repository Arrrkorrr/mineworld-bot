clear

mkdir -p build
cd build

mkdir -p cmake
mkdir -p out
cd cmake

cmake ../../
make

mv bot ../out
cd ../out

cp ../../config/bot.config ./bot.config
chmod +x bot

echo ""
./bot
