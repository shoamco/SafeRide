import ssl
import os
import smtplib
from email.mime.text import MIMEText
from email.mime.image import MIMEImage
from email.mime.multipart import MIMEMultipart

def SendMail(ImgFileName,email):
    with open(ImgFileName, "rb") as attachment:
        img_data = attachment.read()
    msg = MIMEMultipart()
    port = 465  # For SSL
    smtp_server = "smtp.gmail.com"
    sender_email = "hackaton.develop@gmail.com"  # Enter your address
    receiver_email = email # Enter receiver address
    password = "313611352"

    msg['Subject'] = 'image send from SafeRide'
    msg['From'] = sender_email
    msg['To'] = receiver_email
    text = MIMEText("Hi! We saw you riding without an helmet today.\nfor your safety- please note it.\nSafeRide:)\nattached your photo.")
    msg.attach(text)
    image = MIMEImage(img_data, name=os.path.basename(ImgFileName))
    msg.attach(image)
    context = ssl.create_default_context()

    with smtplib.SMTP_SSL(smtp_server, port, context=context) as server:
        server.login(sender_email, password)
        server.sendmail(sender_email, receiver_email, msg.as_string())


