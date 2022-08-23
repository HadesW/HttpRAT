import os
import re
import requests
from http.server import HTTPServer, SimpleHTTPRequestHandler

# 获取外网IP
def get_outter_ip():
    ip = ''    
    try:
        res = requests.get('https://myip.ipip.net', timeout=5).text
        ip = re.findall(r'(\d+\.\d+\.\d+\.\d+)', res)
        ip = ip[0] if ip else ''
    except:
        pass
    return ip


# 服务器工作目录
work_dir = os.getcwd()

# 服务器地址
host = ('0.0.0.0')
port = 8000


if __name__ == '__main__':

    is_start = False

    # 打印工作目录
    print("work dir = ",work_dir)

    # 列出工作目录所有文件
    print("all files in the working directory : ")
    print("====================================")
    for name in os.listdir(work_dir):
        # 存在文件
        if name == "shellcode.bin.txt" or name == "payload.exe.txt":
           name =" *  "+name
           is_start = True
        else:
           name ="    "+name
        print(name)
    print("====================================")

    #
    print("http-srv-py starting...")
    print("listing address= ",host,":",port)
    print("outter address = ",get_outter_ip(),":",port)
    print("...")

    server = HTTPServer((host, port), SimpleHTTPRequestHandler)
    server.serve_forever()