from bs4 import BeautifulSoup
import re
import os

def process_html(fname):
    print fname
    f=open(fname, "r")
    content = f.read()
    f.close()

    #while "scr\"+\"ipt" in content:
    #    content= content.replace('scr\"+\"ipt', 'script')

    out_f = fname[:-5] + ".txt"
    f = open(out_f, "w")
    
    soup = BeautifulSoup(content, "html5lib")
    out_c = str(soup.h1)[4:-5].strip() + "\n\n"

    for d in soup.find_all(id=re.compile("mod-a-body")):
        for p in d.find_all("p"):
            out_c = out_c +  str(p)[3:-4] + "\n\n"
  
    f.write(out_c)
    f.close()



def process_all():
    for f in os.listdir("."):
        if f.endswith(".html"):
            process_html(f)


process_all()
