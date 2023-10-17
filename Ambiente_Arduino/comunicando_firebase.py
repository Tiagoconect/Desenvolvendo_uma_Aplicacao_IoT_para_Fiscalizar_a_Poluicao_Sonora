import pyrebase

# Configuração do Firebase
config = {
    "apiKey": "YOUR_API_KEY",
    "authDomain": "YOUR_AUTH_DOMAIN",
    "databaseURL": "YOUR_DATABASE_URL",
    "projectId": "YOUR_PROJECT_ID",
    "storageBucket": "YOUR_STORAGE_BUCKET",
    "messagingSenderId": "YOUR_MESSAGING_SENDER_ID",
    "appId": "YOUR_APP_ID"
}

firebase = pyrebase.initialize_app(config)
db = firebase.database()

# Recuperar dados do Firebase
data = db.get().val()

# Faça a análise dos dados
# Por exemplo, você pode percorrer os dados e imprimir ou salvá-los em um arquivo
for key, value in data.items():
    print(f"Chave: {key}, Valor: {value}")
    # Você também pode salvar esses dados em um arquivo de texto
    with open("dados.txt", "a") as file:
        file.write(f"Chave: {key}, Valor: {value}\n")
