import subprocess


# used for testing code

export PATH=$PATH:/afs/unity.ncsu.edu/users/z/zwmurray/ece563/Project1

subprocess('make clean')
subprocess('make')

subprocess('./sim_cache 16 1024 2 0 0 0 gcc_trace.txt > my.txt')
subprocess('diff -iw my.txt gcc.output0.txt')


"""
list = [1,2,4,8,0]
list2 = [1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576]
for y in list: 
  for x in list2:
    subprocess('./sim_cache 32 '+ str(x) + ' ' + str(y) + ' 0 0 0 gcc_trace.txt > ./runs/t1L1_size' + str(x) + 'L1_assoc_' + str(y) + '.txt')
print("done")





#os.system(cmd)"""

