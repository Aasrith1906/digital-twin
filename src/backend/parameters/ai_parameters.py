import os
from exceptions.openai_key_not_found import OpenAIKeyNotFoundError


'''
if not 'OPEN_AI_KEY' in os.environ: 
    raise OpenAIKeyNotFoundError()
'''

OPENAI_KEY = os.environ['OPEN_AI_KEY']