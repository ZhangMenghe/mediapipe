fname = "data.txt"
content = open(fname)
lines = content.readlines()
total = 0
err = 0
for line in lines:  
    num = line.split(' ')  
    if(len(num)>1):  
        total+=int(num[-1])
    else:
        err+=1
print(total / (len(lines)-err)) 