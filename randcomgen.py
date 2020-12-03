import os
import random
import string

maxint = 9223372036854775807
minint = 0
counter = 0
looptime = int(input('looptime:'))
letter = string.ascii_letters + string.digits
with open('./test.input', 'w+') as f:
	while counter < looptime - 1:
		rcomflag = random.randint(0,2)
		value = ''
		if(rcomflag == 0):
			key = random.randint(minint,maxint)
			for i in range(128):
				value = value + random.choice(letter)
			content = 'PUT ' + str(key) + ' ' + value
		elif(rcomflag == 1):
			key = random.randint(minint,maxint)
			content = 'GET ' + str(key)
		elif(rcomflag == 2):
			key1 = random.randint(minint,maxint - 100)
			key2 = key1 + random.randint(1,100)
			content = 'SCAN ' + str(key1) + ' ' + str(key2)
		f.write(str(content) + os.linesep)
		if counter % 1000000 == 0:
			print(counter)
		counter = counter + 1

	#last line no need \n
	rcomflag = random.randint(0,2)
	value = ''
	if(rcomflag == 0):
		key = random.randint(minint,maxint)
		for i in range(128):
			value = value + random.choice(letter)
		content = 'PUT ' + str(key) + ' ' + value
	elif(rcomflag == 1):
		key = random.randint(minint,maxint)
		content = 'GET ' + str(key)
	elif(rcomflag == 2):
		key1 = random.randint(minint,maxint - 100)
		key2 = key1 + random.randint(1,100)
		content = 'SCAN ' + str(key1) + ' ' + str(key2)
	f.write(str(content))
f.close()
