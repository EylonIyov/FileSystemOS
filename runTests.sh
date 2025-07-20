echo -e "\033[34mDont forget to thank Omer Goldstein and Eylon Iyov in bit\033[0m"

echo

sleep 1

mkdir "test_imgs"

gcc create_empty_file.c -o create_empty.o 

./create_empty.o

gcc generate_invalid_images.c -o gen_images.o  

./gen_images.o

gcc Test1.c fs.c -o Test1.o 

sleep 2

./Test1.o

gcc Test2.c fs.c -o Test2.o 

sleep 2

./Test2.o

sleep 5

rm Test2.o test_file_system_read.img test_file_system_write.img test_file_system.img Test1.o empty_file.img create_empty.o gen_images.o

rm -r test_imgs test_images

echo

echo -e "\033[34mIf this worked - Bit: 0543970147 / 0509582368\033[0m"
