f = open('answer.output','r')
inputlist = []
for item in f:
    inputlist.append(item)
f.close()

inputf = input("file name:")

f = open(str(inputf) + ".output",'r')
outputlist = []
for item in f :
    outputlist.append(item)
f.close()

if inputlist == outputlist:
    print('correct')
else:
    print('something wrong')
    i = 0
    while i < len(inputlist):
        if inputlist[i] != outputlist[i]:
            print('wrong answer at line %d' %(i + 1))
            print('The answer is %s' %inputlist[i])
            print('your answer is %s' %outputlist[i])
        i = i + 1
