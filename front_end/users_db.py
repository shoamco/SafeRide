from twilio.rest import Client
import os
from flask import Flask, request, render_template, url_for, flash, redirect
from flask_mongoengine import MongoEngine
import mongoengine as me
import base64

################ forms ################
from flask_wtf import FlaskForm
from wtforms import StringField, PasswordField, SubmitField, BooleanField, IntegerField
from wtforms.validators import DataRequired, Length, Email, EqualTo

app = Flask(__name__)
app.config['SECRET_KEY'] = '5791628bb0b13ce0c676dfde280ba245'

app.config['FLASK_ENV'] = True  # development
app.config['FLASK_APP'] = "users_db.py"
app.config['MONGODB_SETTINGS'] = {
    "db": "users_info",
}

db = MongoEngine(app)


class RegistrationForm(FlaskForm):
    user_id = IntegerField("Id", validators=[DataRequired()])

    name = StringField('Name',
                       validators=[DataRequired(), Length(min=2, max=20)])

    email = StringField('Email',
                        validators=[DataRequired(), Email()])

    phone = IntegerField("Phone", validators=[DataRequired()])

    password = PasswordField('Password', validators=[DataRequired()])

    confirm_password = PasswordField('Confirm Password',
                                     validators=[DataRequired(), EqualTo('password')])

    submit = SubmitField('Sign Up')


class LoginForm(FlaskForm):
    user_id = StringField('User Id',
                          validators=[DataRequired()])
    password = PasswordField('Password', validators=[DataRequired()])
    remember = BooleanField('Remember Me')
    submit = SubmitField('Login')


class User(me.Document):
    user_id = me.IntField(unique=True)
    name = me.StringField()
    email = me.StringField()
    password = me.StringField()
    phone = me.IntField()
    num_trips = me.IntField()
    positive_points = me.IntField()
    negative_points = me.IntField()


@app.route('/user_window/<current_user_id>', methods=['GET', 'POST'])
def user_window(current_user_id):
    user = User.objects(user_id=int(current_user_id))
    if user:
        user = user.get(user_id=int(current_user_id))
        calc_points = user.positive_points - user.negative_points
        if calc_points > 0:
            return render_template("user_interface2.html", num=calc_points % 10, name=user.name)
        else:
            return render_template("user_interface2.html", num=0, name=user.name)


@app.route('/add_user', methods=['POST', 'GET'])
def add_user():
    if request.method == 'POST':
        user_id = request.form.get('user_id')
        name = request.form.get('name')
        email = request.form.get('email')
        password = request.form.get('password')
        phone = request.form.get('phone')
        user = User(user_id=user_id, name=name, email=email, password=password, phone=phone, num_trips=0,
                    positive_points=0, negative_points=0)
        user.save()
        return f"hello {user_id},  {name} !"
    return "NOT GET OT POST METHOD WERE USED"


def get_name_and_phone_by_user_id(user_id):
    user = User.objects(user_id=int(user_id))
    user = user.get(user_id=int(user_id))
    return user.name, user.phone


def sendMessage(user_id):
    name, phone = get_name_and_phone_by_user_id(user_id)
    try:
        client = Client(os.environ['TWILIO_ACCOUNT_SID'], os.environ['TWILIO_AUTH_TOKEN'])

        number_dest = f"+972{phone}"
        message = client.messages \
            .create(
            body=f'Hi {name}! we found you rode without helmet today. for your safety- please note it. SafeRide:) ',
            from_='+14805264302',
            to=number_dest
        )
    except:
        print("Failed to send SMS")


@app.route('/update_user', methods=['POST'])
def update_user():
    if request.method == 'POST':
        user_id = request.form.get('user_id')
        is_positive = request.form.get('is_positive')
        user = User.objects(user_id=user_id)
        print(f'user:{user} is_positive:{is_positive}')
        if user:
            user = user.get(user_id=user_id)
            user.num_trips += 1
            print(f'is_positive: {is_positive}, user_id: {user_id}')
            if int(is_positive) == 1:
                user.positive_points += 1
            else:
                user.negative_points += 1
                sendMessage(user_id)

        user.save()
        return f"hello {user_id}, {is_positive} !"
    return "NOT GET OT POST METHOD WERE USED"


# @app.route('/')
@app.route('/users')
def users_table_show():
    return render_template("index.html", users=User.objects())


# <body style="background-color:#E6E6FA">
@app.route("/register", methods=['GET', 'POST'])
def register():
    form = RegistrationForm()
    if form.validate_on_submit():
        flash(f'Account created for {form.name.data}!', 'success')
        user_id = form.user_id.data
        name = form.name.data
        email = form.email.data
        phone = form.phone.data
        password = form.password.data
        user = User(user_id=user_id, name=name, email=email, password=password, phone=phone, num_trips=0,
                    positive_points=0, negative_points=0)
        user.save()
        return redirect(url_for('login'))
    return render_template('register.html', title='Register', form=form)


@app.route("/login", methods=['GET', 'POST'])
def login():
    form = LoginForm()
    if form.validate_on_submit():
        user_id = form.user_id.data
        user = User.objects(user_id=user_id)
        if user:
            user = user.get(user_id=user_id)
            password = user.password
            if form.password.data == password:
                return user_window(user_id)
        else:
            flash('Not Found!', 'danger')
            return render_template('login.html', title='Login', form=form)
    return render_template('login.html', title='Login', form=form)


@app.route("/upload_image", methods=["POST"])
def upload_image():
    if request.method == 'POST':
        jsn = request.get_json()
        img = jsn['img']
        img_name = jsn['img_name']
        with_helmet = jsn['with_helmet']  # send email
        imgdata = base64.b64decode(img)

        if with_helmet == 0:
            try:
                client = Client(os.environ['TWILIO_ACCOUNT_SID'], os.environ['TWILIO_AUTH_TOKEN'])

                message = client.messages \
                    .create(
                    body='היי שהם. מצאנו אותך רוכבת ללא קסדה! אנא היזהרי עבור בטיחותך לפעם הבאה:)',
                    from_='+14805264302',
                    # media_url=['C:\\excellenteam\\downloads\\helmet.jpg'],
                    to='+9720526064628'
                )
            except:
                print("Failed to send SMS")

        with open(f'images\\{img_name}.jpg', 'wb') as f:
            f.write(imgdata)
        return "got the image!"


if __name__ == '__main__':
    app.run("0.0.0.0")
    # app.run()
