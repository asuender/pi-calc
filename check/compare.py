#!/usr/bin/python3
import os,sys
orig=open("pi")
comp=open("_pi")
comptxt=comp.read()
l=len(comptxt)
origtxt=orig.read(l)
print(l," Stellen...")
errs=[]
for i,ch in enumerate(comptxt):
	if ch!=origtxt[i]:
		errs.append((i,ch,origtxt[i]))
print("There were %s errs in positions:"%(len(errs)))
for err in errs:
	print("%10i:	%s→%s"%err)
