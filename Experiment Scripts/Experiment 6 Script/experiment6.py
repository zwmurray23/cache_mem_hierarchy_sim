import os


# used for testing code

# export PATH=$PATH:/afs/unity.ncsu.edu/users/z/zwmurray/ece563/Project1

os.system('make clean')
os.system('make')

os.system('./sim_cache 16 1024 2 0 0 0 gcc_trace.txt > my.txt')
os.system('diff -iw my.txt gcc.output0.txt')



list = [0, 2, 4, 8, 16] #maps to str(y), VC sizes
list2 = [1024, 2048, 4096, 8192, 16384, 32768] #maps to str(x), L1 sizes
for y in list: 
  for x in list2:
    os.system('./sim_cache ' + '32' + ' ' + str(x) + ' 1 ' + str(y) + ' 65536 ' + '8 gcc_trace.txt > ./runs/e6_Direct_mapped_L1_Size_' + str(x) + '_VC_size_' + str(y) + '.txt')


list = [2, 4] #maps to str(y), L1assoc sizes
list2 = [1024, 2048, 4096, 8192, 16384, 32768] #maps to str(x), L1 sizes
for y in list: 
  for x in list2:
    os.system('./sim_cache ' + '32' + ' ' + str(x) + ' ' + str(y) + ' 0' + ' 65536 ' + '8 gcc_trace.txt > ./runs/e6_L1_Size_' + str(x) + '_L1_assoc_' + str(y) + '_noVC.txt')


print("done")





# os.system(cmd)

