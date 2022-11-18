printf "_________________________________________________________________\n"
sleep 0.1
if [ "$2" != '' ]; then
	if [ "$1" == "-embed" ]; then
		printf "[+] Adding Cleny to $2\n"
		sleep 1
		cp ../Cleny "$2" -r
		printf "[+] Cleny Added To $2 Successfully\n"
		exit 0
	fi
fi
sleep 0.1
if [ $(whoami) != "root" ] && [ "" == "" ]; then
	printf "ERROR: Sudo Privileges Required To Install Cleny On Your System\n"
	sleep 0.1
	printf "Please Run 'sudo bash install.sh'\n"
	sleep 0.1
	printf "\nDid You Mean \`./install.sh -embed <Folder>\` ?\n"
	exit 1
fi

printf "[+] Compiling Binaries ...\n"
sleep 0.5
g++ cleny/cleny.cpp -o /bin/cleny &> /dev/null

printf "[i] https://github.com/Mahmoud7Osman/Cleny\n"
sleep 1
printf "[i] in the README.md section, You will find everything you need\n\n"
sleep 1
printf "[<] Thank You For Installing Cleny\n"
sleep 0.5
printf "_________________________________________________________________\n"


if [ ! -f "/bin/cleny" ]; then
	cp bin/x64_cleny /bin/cleny
	exit 0
fi
