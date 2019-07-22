import requests
from bs4 import BeautifulSoup
from random import *
import pymysql
import threading
import time

conn1=pymysql.connect(host="xxx.xxx.xx.xx",port=OOOOO,user="OOOOO",passwd="OOOO",db="OOOOO",charset="utf8")
cur1=conn1.cursor(pymysql.cursors.DictCursor)

conn2=pymysql.connect(host="xxx.xxx.xx.xx",port=OOOOO,user="OOOOO",passwd="OOOO",db="OOOOO",charset="utf8")
cur2=conn2.cursor(pymysql.cursors.DictCursor)

url='http://web.kma.go.kr/aboutkma/intro/gwangju/index.jsp'
source_code=requests.get(url)

plain_text=source_code.text

soup=BeautifulSoup(plain_text,'lxml')
sign=[1,-1]

def wind(windDir):
    if windDir=='북풍':
        return 0
    elif windDir=='북동풍':
        return 2
    elif windDir=='동풍':
        return 4
    elif windDir=='남동풍':
        return 6
    elif windDir=='남풍':
        return 8
    elif windDir=='남서풍':
        return 10
    elif windDir=='서풍':
        return 12
    elif windDir=='북서풍':
        return 14

def addData(direction, speed, rain):
    if float(speed)<0:
        speed=(float(speed)*(-1))
    query="insert into weather_far(direction,speed,rain) values("+str(direction)+","+str(int(speed))+","+str(rain)+")"
    cur1.execute(query)
    cur2.execute(query)
    conn1.commit()
    conn2.commit()
    print('end')

def getData():
    weather=soup.find_all('div',{'class':'wrap_weather_info_jibang '})
    if len(weather)==0:
        weather=soup.find_all('div',{'class':'wrap_weather_info_jibang wrap_weather_info_jibang_new'})
    #print(len(weather))
    weather=weather[0].getText()

    windDir=weather.split('바람 : ')[1]
    windDir=windDir.split(' ')[0]
    sqlDir=wind(windDir)
    randDir=randint(-3,3)
    sqlDir=(sqlDir+16+randDir)%16

    windSpeed=weather.split('바람 : ')[1]
    windSpeed=windSpeed.split(' ')[1]
    windSpeed=float(windSpeed.split('m/s')[0])
    randSign=choice(sign)    
    randDir=round((randSign*random()*2),2)
    sqlSpeed=round(windSpeed+randDir,2)

    rain=weather.split('1시간 강수량 : ')[1].split('\n')[0]
    if rain=='-':
        rain=0
    else:
        rain=rain.split('mm')[0]
        rain=float(rain)

    addData(sqlDir,sqlSpeed,rain)
    

def getDataThread():
    getData()
    threading.Timer(5,getDataThread).start()

getDataThread()
