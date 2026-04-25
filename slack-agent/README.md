# Slack Agent Server

Node.js Slack bot server for talking to an AI assistant from Slack.

The bot uses Slack Socket Mode, so local development does not need a public
webhook URL.

## Required Slack tokens

Create or open the Slack app, then configure:

1. **Socket Mode**
   - Enable Socket Mode.
   - Create an app-level token with `connections:write`.
   - Put it in `SLACK_APP_TOKEN`.
   - This token starts with `xapp-`.

2. **OAuth & Permissions**
   - Add Bot Token Scopes:
     - `chat:write`
     - `app_mentions:read`
     - `im:history`
     - `im:write`
     - `channels:history` if the bot should read public channel messages.
     - `groups:history` if the bot should read private channel messages.
   - Install or reinstall the app to the workspace.
   - Put the Bot User OAuth Token in `SLACK_BOT_TOKEN`.
   - This token starts with `xoxb-`.

3. **Event Subscriptions**
   - Subscribe to bot events:
     - `app_mention`
     - `message.im`

## AI token

Set `OPENAI_API_KEY` to the API key used by the OpenAI SDK.

Optional:

- `OPENAI_MODEL`, default: `gpt-4.1-mini`
- `OPENAI_BASE_URL`, if using an OpenAI-compatible endpoint

## Run

```bash
cp .env.example .env
npm install
npm start
```

Then in Slack:

- Mention the bot in a channel: `@your-bot hello`
- Or DM the bot directly.

## Notes

- Do not commit `.env`.
- Keep `xapp-`, `xoxb-`, and AI API keys secret.
