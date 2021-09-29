import os


# used for testing code

# export PATH=$PATH:/afs/unity.ncsu.edu/users/z/zwmurray/ece563/Project1

os.system('make clean')
os.system('make')

os.system('./sim_cache 16 1024 2 0 0 0 gcc_trace.txt > my.txt')
os.system('diff -iw my.txt gcc.output0.txt')


#No L2, L1 associativity is 4
list = [16,32,64,128] #maps to str(y)
list2 = [1024, 2048, 4096, 8192, 16384, 32768] #maps to str(x)
for y in list: 
  for x in list2:
    os.system('./sim_cache ' + str(y) + ' ' + str(x) + ' 4 0 0 0 gcc_trace.txt > ./runs/e4_L1_blocksize_' + str(y) + '_L1_size_' + str(x) + '.txt')
print("done")





# os.system(cmd)

