file1 = open("testfile.txt","w")


for i in range(1000):
    file1.write("This is a write")

file1.close()

file2 = open("testfile.txt","r")
file3 = open("testfile2.txt","w")
file4 = open("testfile.txt","r")
string = ""
cycle = 0


for line in file2:
    for line in file4:
        file3.write("hi")
    string += line
    cycle += 1
    if (cycle % 100) == 0:
        file3.write(string)
        string = ""

file2.close()
file3.close()
file4.close()
