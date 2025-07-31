README :- SSD1306 0.91 OLED Display
This is a simple C Program code that Helps you Print Time on the Oled. The first version displays time in HH::MM::SS format.
In later versions ill include the freedom of choosing fonts, animations, formats etc, by including headers and custom fonts made by editing the hex code.
I've als include the arm binary file to directly convert to executable on the luckfox.


prequisites:

1. Luckfox Pico (Mini B) connected over USB/LAN.
2. Cross-compiler toolchain installed (arm-linux-gnueabihf-gcc).
3. IP address of the Luckfox board.
4. SSH/SCP access enabled on the board.


Steps : ( This is done in the host machine, in my case its an ubuntu linux machine and my target is an ARM based SoC)

1. Navigate to Desired Directory using "cd home/your/path" .
2. Create the c file using " nano filename.c " write/paste the above code using nano terminal and press ctrl+o (to save) and ctrl+x (to exit).
3. Compile the code for arm based target i.e luckfox using i.e "arm-linux-gnueabihf-gcc -static filename.c -o filename"(to be created)
   I use -static as my target OS is minimal buildroot and lacks dynamic libraries. The command converts the c code to ARM based C file.
4. Then using using SCP command transfer the ARM_C file to luckfox pico using the command "SCP filesname root@<luckfoxip>:/<dest. addr>.

(connect the luckfox device on the host machine and get the ip address of the device)

5. Login to the Luckfox using ssh, i.e "ssh root@ip address"
6. Change directory to the dest. addr. then using chmod +x filename change the file permission i.e change the arm_c file to executable to run code on luckfox.
7. Then using command ./filename run the executable on luckfox.
