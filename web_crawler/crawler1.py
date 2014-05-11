import os
import sys
import datetime
import time


def generate_js_file(date, page=1):
	page_str = ''
	if page > 1:
		page_str = str(page) + '/'
	prefix = "var page = require('webpage').create();" + \
	         "page.settings.userAgent = 'SpecialAgent';" + \
	         "page.open(\'http://query.nytimes.com/search/sitesearch/" + \
	                    "?action=click&region=Masthead&pgtype=Homepage&" + \
	                    "module=SearchSubmit&contentCollection=Homepage&t=" + \
	                    "qry201#/china/from"
	prefix = prefix + date + "to" + date + "/articles/" + page_str + "\', function(status) {" + \
             "if (status !== \'success\') {" + \
             "console.log(\'Unable to access network\');" + \
             "} else {" + \
             "var ua = page.evaluate(function() {" + \
             "var ret = [];" + \
             "var lists = document.querySelectorAll(\"li.story\");" + \
             "for (i = 0; i < lists.length; i++) {"	 + \
             "var matches = lists[i].querySelectorAll(\'a\');" + \
             "if(matches.length==1 ){" + \
             "ret.push(matches[0].innerText);" + \
             "ret.push(matches[0].getAttribute(\"href\"));" + \
             "}" + \
             "if(matches.length==2 ){" + \
             "ret.push(matches[1].innerText);" + \
             "ret.push(matches[1].getAttribute(\"href\"));" + \
             "}" + \
             "}" + \
             "return ret;" + \
             "});" + \
             "console.log(ua.join(\'\\n\'));" + \
             "}" + \
             "phantom.exit();" + \
             "});"
	
	f = open('js/' + date + "_" + str(page) + ".js", "w")
	f.write(prefix + '\n')
	f.close()


def fetch_one_day(date):
	page = 1
	os.system('echo ' + '[' + date + ']' + ' >> all.out')
	retry = 3
	err_log = open("err.log", "a")
	success = False
	while retry > 0:
		generate_js_file(date, page)
		js_name = 'js/' + date + '_' + str(page) + '.js'
		print 'bin/phantomjs ' + js_name + ' >  res.out'
		os.system('bin/phantomjs ' + js_name + ' >  res.out')		
		f = open('res.out', 'r')
		lines = f.readlines()
		if len(lines) == 20:
			os.system('cat res.out >> all.out')
			page = page + 1
			success = True
		elif len(lines) == 1:
			retry = retry - 1
		else:
			os.system('cat res.out >> all.out')
			success = True
			break

	if not success:
		err_log.write("[Error]: " + date + "\n")

	err_log.close()


def go():
	start_year = 2010
	start_mon = 2
	start_day = 2
	end_year = 2014
	cur = datetime.date(start_year, start_mon, start_day)
	while cur.year < end_year:
		mon_str = ''
		day_str = ''
		if cur.month < 10:
			mon_str = '0' + str(cur.month)
		else:
			mon_str = str(cur.month)

		if cur.day < 10:
			day_str = '0' + str(cur.day)
		else:
			day_str = str(cur.day)

		fetch_one_day(str(cur.year) + mon_str + day_str)
		cur = cur + datetime.timedelta(1)
		time.sleep(2)


go()




