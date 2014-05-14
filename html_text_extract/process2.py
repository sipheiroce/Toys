from bs4 import BeautifulSoup
import re
import os

def process_html(fname):
    print fname
    f=open(fname, "r")
    content = f.read()
    f.close()

    out_f = fname[:-5] + ".txt"
    f = open(out_f, "w")
    
    soup = BeautifulSoup(content, "html5lib")
    out_c = str(soup.h1.text) + "\n\n"

    for dp in soup.find_all("p", class_="summaryText"):
        out_c = out_c + str(dp.text) + "\n\n"


    for d in soup.find_all(id=re.compile("mod-a-body")):
        for p in d.find_all("p"):
            out_c = out_c +  str(p)[3:-4] + "\n\n"

    for dp in soup.find_all("div", class_="articleBody"):
        for p in dp.find_all("p"):
            out_c = out_c + p.getText().encode('utf-8').strip() + "\n\n"
  
    f.write(out_c)
    f.close()



def process_all():
    for f in os.listdir("."):
        if f.endswith(".html"):
            process_html(f)


process_all()
