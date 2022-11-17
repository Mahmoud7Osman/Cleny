EMBED=''
printf "_________________________________________________________________\n"
sleep 0.1
if [ "$2" != '' ]; then
	if [ "$1" == "-embed" ]; then
		printf "[>] Installing Cleny To $2\n\n"
		mkdir "$2"/bin &>/dev/null
		EMBED="$2"
	fi
fi
sleep 0.1
if [ $(whoami) != "root" ] && [ "$EMBED" == "" ]; then
	printf "ERROR: Sudo Privileges Required To Install Cleny On Your System\n"
	sleep 0.1
	printf "Please Run 'sudo bash install.sh'\n"
	sleep 0.1
	printf "\nDid You Mean \`./install.sh -embed <Folder>\` ?\n"
	exit 1
fi

printf "[+] Compiling Binaries ...\n"
sleep 0.5
g++ cleny/cleny.cpp -o $EMBED/bin/cleny &> /dev/null

printf "[i] https://github.com/Mahmoud7Osman/Cleny\n"
sleep 1
printf "[i] in the README.md section, You will find everything you need\n\n"
sleep 1
printf "[<] Thank You For Installing Cleny\n"
sleep 0.5
printf "_________________________________________________________________\n"


if [ ! -f "bin/cleny" ]; then
	cp bin/x64_cleny $EMBED/bin/cleny
	exit 0
fi
cp bin/cleny $EMBED/bin/cleny
