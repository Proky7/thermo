FROM python:3.11-slim

WORKDIR /app

# Závislosti instalujeme nejdřív — využití Docker cache
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Aplikační kód
COPY app/ ./app/

# Nepouštět jako root
RUN useradd -r -u 1000 appuser
USER appuser

EXPOSE 8000

CMD ["uvicorn", "app.main:app", "--host", "0.0.0.0", "--port", "8000"]