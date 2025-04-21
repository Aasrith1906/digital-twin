

class OpenAIKeyNotFoundError(Exception): 
    
    def __init__(self):
        super().__init__("OpenAI API Key not found, to use store the OpenAI Key in OPEN_AI_KEY")   