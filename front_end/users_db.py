from flask import Flask, request, render_template
from flask_mongoengine import MongoEngine
import mongoengine as me

app = Flask(__name__)

app.config['FLASK_ENV'] = True  # development
app.config['FLASK_APP'] = "users_db.py"
app.config['MONGODB_SETTINGS'] = {
    "db": "users_info",
}

db = MongoEngine(app)


class User(me.Document):
    user_id = me.IntField(unique=True)
    name = me.StringField()
    positive_points = me.IntField()
    negative_points = me.IntField()


"""@app.route('/')
def hello_world():
    user = User(user_id=122, num_travels=30, name='tamar')
    user.save()
    return 'Hello, World! TeamUP'"""


@app.route('/add_user', methods=['POST', 'GET'])
def add_user():
    if request.method == 'POST':
        user_id = request.form.get('user_id')
        name = request.form.get('name')
        user = User(user_id=user_id, name=name, positive_points=0, negative_points=0)
        user.save()
        return f"hello {user_id},  {name} !"
    return "NOT GET OT POST METHOD WERE USED"


@app.route('/update_user', methods=['POST'])
def update_user():
    if request.method == 'POST':
        user_id = request.form.get('user_id')
        is_positive = request.form.get('is_positive')
        user = User.objects(user_id=user_id)
        if user:
            user = user.get(user_id=user_id)
            if int(is_positive) == 1:
                user.positive_points += 1
            else:
                user.negative_points += 1

        user.save()
        return f"hello {user_id}, {is_positive} !"
    return "NOT GET OT POST METHOD WERE USED"


@app.route('/')
@app.route('/users')
def users_table_show():
    return render_template("index.html", users=User.objects())


if __name__ == '__main__':
    app.run("0.0.0.0")
