from telegram import Update
from telegram.ext import Updater, CommandHandler, MessageHandler, Filters, CallbackContext

# Replace 'YOUR_BOT_TOKEN' with the token you obtained from BotFather
TOKEN = 'YOUR_BOT_TOKEN'

def start(update: Update, context: CallbackContext) -> None:
    update.message.reply_text('Hello! I am your bot. Use /echo to echo a message.')

def echo(update: Update, context: CallbackContext) -> None:
    # Parse additional arguments
    args = context.args
    if not args:
        update.message.reply_text('Please provide a message to echo.')
    else:
        message = ' '.join(args)
        update.message.reply_text(f'Echo: {message}')

def main() -> None:
    updater = Updater(TOKEN)

    dp = updater.dispatcher

    # Add command handlers
    dp.add_handler(CommandHandler("start", start))
    dp.add_handler(CommandHandler("echo", echo, pass_args=True))

    updater.start_polling()

    updater.idle()

if __name__ == '__main__':
    main()
