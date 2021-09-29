import os


# used for testing code

# export PATH=$PATH:/afs/unity.ncsu.edu/users/z/zwmurray/ece563/Project1

os.system('make clean')
os.system('make')

os.system('./sim_cache 16 1024 2 0 0 0 gcc_trace.txt > my.txt')
os.system('diff -iw my.txt gcc.output0.txt')



list = [1,2,4,8,0]
list2 = [1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144]
for y in list: 
  for x in list2:
    os.system('./sim_cache 32 '+ str(x) + ' ' + str(y) + ' 0 524288 8 gcc_trace.txt > ./runs/e1_L1_size_' + str(x) + '_L1_assoc_' + str(y) + '.txt')
print("done")





# os.system(cmd)

