import requests
post_data = {'user_id': '44343434','is_positive':1}
ans = requests.post("http://10.144.68.95:5000/update_user",data=post_data)