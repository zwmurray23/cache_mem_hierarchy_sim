import os


# used for testing code

# export PATH=$PATH:/afs/unity.ncsu.edu/users/z/zwmurray/ece563/Project1

os.system('make clean')
os.system('make')

os.system('./sim_cache 16 1024 2 0 0 0 gcc_trace.txt > output0.txt')
os.system('diff -iw output0.txt gcc.output0.txt')

os.system('./sim_cache 16 1024 2 16 0 0 gcc_trace.txt > output1.txt')
os.system('diff -iw output1.txt gcc.output1.txt')

os.system('./sim_cache 16 1024 2 0 8192 4 gcc_trace.txt > output2.txt')
os.system('diff -iw output2.txt gcc.output2.txt')

os.system('./sim_cache 16 1024 2 16 8192 4 gcc_trace.txt > output3.txt')
os.system('diff -iw output3.txt gcc.output3.txt')

os.system('./sim_cache 16 1024 1 0 0 0 gcc_trace.txt > output4.txt')
os.system('diff -iw output4.txt gcc.output4.txt')

os.system('./sim_cache 16 1024 1 16 0 0 gcc_trace.txt > output5.txt')
os.system('diff -iw output5.txt gcc.output5.txt')

os.system('./sim_cache 16 1024 1 0 8192 4 gcc_trace.txt > output6.txt')
os.system('diff -iw output6.txt gcc.output6.txt')

os.system('./sim_cache 16 1024 1 16 8192 4 gcc_trace.txt > output7.txt')
os.system('diff -iw output7.txt gcc.output7.txt')





print("done")





# os.system(cmd)

