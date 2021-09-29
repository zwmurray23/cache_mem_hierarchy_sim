import os


# used for testing code

# export PATH=$PATH:/afs/unity.ncsu.edu/users/z/zwmurray/ece563/Project1

os.system('make clean')
os.system('make')

os.system('./sim_cache 16 1024 2 0 0 0 gcc_trace.txt > my.txt')
os.system('diff -iw my.txt gcc.output0.txt')



list = [32768, 65536, 131072, 262144, 524288, 1048576] #maps to str(y), L2 sizes
list2 = [1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144] #maps to str(x), L1 sizes
for y in list: 
  for x in list2:
    os.system('./sim_cache ' + '32' + ' ' + str(x) + ' 4 0 ' + str(y) + '8 gcc_trace.txt > ./runs/e5_L1_Size_' + str(x) + '_L2_size_' + str(y) + '.txt')
print("done")





# os.system(cmd)

