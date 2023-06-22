#!/bin/bash
export DEBIAN_FRONTEND=noninteractive
export DEBIAN_PRIORITY=critical

apt-get update

# Leggi le variabili dal file di configurazione
config=$(cat -)

# Check if the temporary directory exists
if [ ! -d "/scratch/temp" ]; then
  mkdir /scratch/temp
fi

# Save the packages and their dependencies to the temporary directory
IFS=',\n' read -ra packages <<< "$config"
for package in "${packages[@]}"; do
  echo "Downloading $package and its dependencies..."
  apt-get -d -o=dir::cache=/scratch/temp --reinstall --yes install "$package"
done

# Install the packages and save the order of installation
for package in "${packages[@]}"; do
  echo "Installing $package and its dependencies..."
  apt-get --simulate -y install "$package" | grep "Inst " | awk '{print $2}' >> /scratch/install_order.txt
  apt-get install -y "$package"
done

# Create a script to install the packages from the temporary directory
echo "#!/bin/bash" >> /scratch/install_from_temp.sh
echo "export DEBIAN_FRONTEND=noninteractive" >> /scratch/install_from_temp.sh
echo "export DEBIAN_PRIORITY=critical" >> /scratch/install_from_temp.sh
echo "" >> /scratch/install_from_temp.sh
echo "for package in \$(cat /scratch/install_order.txt); do" >> /scratch/install_from_temp.sh
echo "  echo \"Installing \$package...\"" >>/scratch/install_from_temp.sh
echo "  dpkg -i /scratch/temp/archives/\$package*.deb" >> /scratch/install_from_temp.sh
echo "done" >> /scratch/install_from_temp.sh
chmod +x /scratch/install_from_temp.sh
chmod 777 -R /scratch/
echo "Installation completed!"

