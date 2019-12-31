from twilio.rest import Client
import os
# Your Account Sid and Auth Token from twilio.com/console
# DANGER! This is insecure. See http://twil.io/secure

client = Client(os.environ['TWILIO_ACCOUNT_SID'], os.environ['TWILIO_AUTH_TOKEN'])

message = client.messages \
    .create(
         body='היי שהם. מצאנו אותך רוכבת ללא קסדה! אנא היזהרי עבור בטיחותך לפעם הבאה:)',
         from_='+14805264302',
         # media_url=['C:\\excellenteam\\downloads\\helmet.jpg'],
         to='+972587885012'
     )
