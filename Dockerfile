FROM python:3.9-slim

WORKDIR /app

# Copia os arquivos do projeto
COPY app.py /app/app.py
COPY templates /app/templates
COPY static /app/static

# Instala o Flask
RUN pip install flask

# Expõe a porta que o Flask usa
EXPOSE 5000

# Comando para rodar a aplicação
CMD ["python", "app.py"]
